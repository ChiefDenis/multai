#include "BrowserTab.h"
#include "BrowserWorkspace.h"
#include "MainWindow.h"
#include "Profile.h"
#include "ProfileManager.h"
#include "ProfileManagementDialog.h"
#include "SettingsManager.h"
#include "Sidebar.h"
#include "TabStore.h"
#include "Theme.h"

#include <QApplication>
#include <QComboBox>
#include <QDir>
#include <QElapsedTimer>
#include <QEventLoop>
#include <QImage>
#include <QPainter>
#include <QPixmap>
#include <QScreen>
#include <QSet>
#include <QStandardPaths>
#include <QTimer>
#include <QWebEngineView>
#include <QWindow>

#include <cstdio>
#include <cstdlib>
#include <functional>

namespace {

constexpr int kWindowWidth = 1366;
constexpr int kWindowHeight = 768;
constexpr int kLoadTimeoutMs = 45000;

QString g_outputDir = "artifacts/visual-qa";
bool g_dark = false;

[[noreturn]] void fail(const QString& message) {
  std::fprintf(stderr, "VISUAL-QA FAILURE: %s\n", qPrintable(message));
  std::fflush(stderr);
  std::exit(2);
}

void require(bool condition, const QString& message) {
  if (!condition) {
    fail(message);
  }
}

bool waitUntil(const std::function<bool()>& condition, int timeoutMs) {
  QElapsedTimer timer;
  timer.start();
  while (!condition()) {
    if (timer.elapsed() > timeoutMs) {
      return false;
    }
    QEventLoop loop;
    QTimer::singleShot(50, &loop, &QEventLoop::quit);
    loop.exec();
  }
  return true;
}

bool isExposed(QWidget* widget) {
  if (!widget || !widget->isVisible()) {
    return false;
  }
  QWindow* handle = widget->windowHandle();
  return handle && handle->isExposed();
}

bool hasRenderedContent(const QPixmap& pixmap) {
  if (pixmap.isNull()) {
    return false;
  }
  const QImage image = pixmap.toImage();
  const QRgb first = image.pixel(0, 0);
  for (int y = 0; y < image.height(); y += 4) {
    for (int x = 0; x < image.width(); x += 4) {
      if (image.pixel(x, y) != first) {
        return true;
      }
    }
  }
  return false;
}

QPixmap captureWindow(MainWindow* window) {
  QPixmap shot(window->size());
  shot.fill(Qt::transparent);
  window->render(&shot);

  QPainter painter(&shot);
  for (QWebEngineView* view : window->findChildren<QWebEngineView*>()) {
    if (!view->isVisible()) {
      continue;
    }
    const QPixmap content = view->grab();
    if (content.isNull()) {
      continue;
    }
    const QPoint position = view->mapTo(window, QPoint(0, 0));
    painter.drawPixmap(QRect(position, view->size()), content);
  }
  painter.end();
  return shot;
}

bool savePixmap(const QPixmap& pixmap, const QString& name) {
  QDir dir(g_outputDir);
  if (!dir.exists() && !dir.mkpath(".")) {
    fail(QString("Cannot create output directory: %1").arg(g_outputDir));
  }
  const QString path = dir.filePath(name);
  if (!pixmap.save(path, "PNG")) {
    fail(QString("Cannot save screenshot: %1").arg(path));
  }
  std::printf("Captured %s (%dx%d)\n", qPrintable(path), pixmap.width(), pixmap.height());
  return true;
}

BrowserTab* waitForLoad(MainWindow* window) {
  BrowserWorkspace* workspace = window->workspaceForTest();
  const bool loaded = waitUntil(
      [window, workspace]() {
        BrowserTab* tab = window->currentTabForTest();
        if (!tab || workspace->tabCount() == 0) {
          return false;
        }
        const LifecycleState state = tab->lifecycleState();
        return state != LifecycleState::Loading && state != LifecycleState::Unloaded;
      },
      kLoadTimeoutMs);
  require(loaded, "Provider page did not leave the Loading/Unloaded state before timeout");

  BrowserTab* tab = window->currentTabForTest();
  require(tab != nullptr, "No active tab after navigation");
  require(tab->lifecycleState() == LifecycleState::Active,
          QString("Navigation did not succeed: %1").arg(lifecycleStateToString(tab->lifecycleState())));

  QWebEngineView* view = tab->view();
  require(view != nullptr, "Active tab has no browser view");
  const bool painted = waitUntil([view]() { return hasRenderedContent(view->grab()); }, 10000);
  require(painted, "Browser view never produced painted content");
  return tab;
}

void openProviderAndWait(MainWindow* window, const QString& providerId) {
  const int before = window->workspaceForTest()->tabCount();
  window->openProviderForTest(providerId);
  require(window->workspaceForTest()->tabCount() == before + 1,
          QString("Provider tab was not created: %1").arg(providerId));
  BrowserTab* tab = waitForLoad(window);
  require(tab->providerId() == providerId,
          QString("Active tab provider mismatch: expected %1, got %2")
              .arg(providerId, tab->providerId()));
}

void configureAccountDialog(ProfileManagementDialog* dialog) {
  ProfileManager* manager = ProfileManager::instance();
  Profile* primary = manager->getOrCreateProfile("deepseek", "default");
  require(primary != nullptr, "Could not create default DeepSeek profile");
  manager->renameProfile(primary, "Default");
  Profile* work = manager->getOrCreateProfile("deepseek", "work");
  require(work != nullptr, "Could not create work DeepSeek profile");
  manager->renameProfile(work, "Work");

  QComboBox* combo = dialog->findChild<QComboBox*>();
  require(combo != nullptr, "Account dialog has no provider selector");
  int deepseekIndex = -1;
  for (int i = 0; i < combo->count(); ++i) {
    if (combo->itemData(i).toString() == "deepseek") {
      deepseekIndex = i;
      break;
    }
  }
  require(deepseekIndex >= 0, "Account dialog does not list the DeepSeek provider");
  combo->setCurrentIndex(deepseekIndex);
  QApplication::processEvents();

  const QString provider = combo->currentData().toString();
  require(provider == "deepseek", "Account dialog provider selection did not change");
  require(manager->profilesForProvider(provider).size() >= 2,
          "Account dialog has no profiles to display");
}

void runState(MainWindow* window, int state, ProfileManagementDialog** dialogOut) {
  BrowserWorkspace* workspace = window->workspaceForTest();
  Sidebar* sidebar = window->sidebarForTest();

  switch (state) {
    case 0:
      require(workspace->tabCount() == 0, "Initial state must not have open tabs");
      require(!sidebar->isCollapsed(), "Initial state must have the sidebar expanded");
      break;

    case 1:
      openProviderAndWait(window, "deepseek");
      require(workspace->tabCount() == 1, "Active state must have exactly one tab");
      require(!sidebar->isCollapsed(), "Active state must have the sidebar expanded");
      require(window->currentTabForTest() != nullptr, "Active state has no selected tab");
      break;

    case 2:
      openProviderAndWait(window, "deepseek");
      openProviderAndWait(window, "qwen");
      openProviderAndWait(window, "gemini");
      require(workspace->tabCount() == 3, "Multiple-tabs state must have exactly three tabs");
      require(!sidebar->isCollapsed(), "Multiple-tabs state must have the sidebar expanded");
      {
        QSet<QString> providers;
        for (const QString& tabId : workspace->tabIds()) {
          BrowserTab* tab = workspace->tabById(tabId);
          require(tab != nullptr, "Workspace returned a null tab");
          providers.insert(tab->providerId());
        }
        require(providers.size() == 3, "Multiple-tabs state must use three distinct providers");
      }
      break;

    case 3:
      openProviderAndWait(window, "deepseek");
      {
        const QSize before = window->size();
        sidebar->setCollapsed(true);
        require(sidebar->isCollapsed(), "Sidebar did not collapse");
        require(sidebar->collapsedRailForTest()->isVisible(), "Collapsed rail is not visible");
        require(window->size() == before, "Collapsing the sidebar changed the window size");
      }
      break;

    case 4:
      require(workspace->tabCount() == 0, "Empty state must not have open tabs");
      require(!sidebar->isCollapsed(), "Empty state must have the sidebar expanded");
      break;

    case 5: {
      auto* dialog = new ProfileManagementDialog(window);
      dialog->setAttribute(Qt::WA_DeleteOnClose);
      configureAccountDialog(dialog);
      dialog->show();
      dialog->raise();
      dialog->activateWindow();
      require(waitUntil([dialog]() { return isExposed(dialog); }, 10000),
              "Account dialog did not become visible/exposed");
      require(dialog->isVisible(), "Account dialog is not visible");
      *dialogOut = dialog;
      break;
    }

    default:
      fail(QString("Unknown visual QA state index: %1").arg(state));
  }
}

}  // namespace

int main(int argc, char* argv[]) {
  QApplication app(argc, argv);
  QStandardPaths::setTestModeEnabled(true);
  app.setApplicationName("AI Workspace Visual QA");

  if (argc < 4) {
    std::fprintf(stderr,
                 "Usage: %s <state 0-5> <light|dark> <output-dir>\n"
                 "States: 0 initial, 1 active, 2 multiple, 3 collapsed, 4 empty, 5 accounts\n",
                 argv[0]);
    return 1;
  }

  const int state = std::atoi(argv[1]);
  g_dark = QString(argv[2]) == "dark";
  g_outputDir = QString::fromUtf8(argv[3]);

  const QString theme = g_dark ? "dark" : "light";
  SettingsManager::instance()->setTheme(theme);
  SettingsManager::instance()->reset("sidebar/collapsed");
  SettingsManager::instance()->reset("sidebar/geometry");
  TabStore::clear();
  Theme::applyTheme(theme);

  auto* window = new MainWindow();
  window->resize(kWindowWidth, kWindowHeight);
  window->show();
  require(waitUntil([window]() { return isExposed(window); }, 10000),
          "Main window did not become visible/exposed");
  window->resize(kWindowWidth, kWindowHeight);
  require(waitUntil(
              [window]() { return window->size() == QSize(kWindowWidth, kWindowHeight); }, 10000),
          "Main window did not reach the requested capture size");
  QApplication::processEvents();

  ProfileManagementDialog* dialog = nullptr;
  runState(window, state, &dialog);

  if (dialog) {
    const QPixmap shot = dialog->grab();
    require(!shot.isNull(), "Account dialog capture failed");
    require(shot.size() == dialog->size() * dialog->devicePixelRatioF(),
            "Account dialog capture size does not match the dialog");
    require(hasRenderedContent(shot), "Account dialog capture is blank");
    savePixmap(shot, QString("%1-%2.png").arg("06-account-management", theme));
    dialog->close();
  } else {
    window->resize(kWindowWidth, kWindowHeight);
    require(waitUntil(
                [window]() { return window->size() == QSize(kWindowWidth, kWindowHeight); }, 10000),
            "Main window changed size before capture");
    QApplication::processEvents();
    const QPixmap shot = captureWindow(window);
    require(!shot.isNull(), "Main window capture failed");
    require(shot.size() == QSize(kWindowWidth, kWindowHeight),
            "Main window capture has an unexpected size");
    require(hasRenderedContent(shot), "Main window capture is blank");

    QString name;
    switch (state) {
      case 0: name = "01-initial"; break;
      case 1: name = "02-active-workspace"; break;
      case 2: name = "03-multiple-tabs"; break;
      case 3: name = "04-sidebar-collapsed"; break;
      case 4: name = "05-empty-workspace"; break;
      default: fail("Unsupported capture state");
    }
    savePixmap(shot, QString("%1-%2.png").arg(name, theme));
  }

  dialog = nullptr;
  window->close();
  delete window;
  app.processEvents();
  std::fflush(stdout);
  std::fflush(stderr);
  // The capture tool exits without static teardown: Qt WebEngine profile
  // destruction after QApplication teardown can abort on some drivers and is
  // unrelated to the captured UI state.
  std::_Exit(0);
}
