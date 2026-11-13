#include "BrowserTab.h"
#include "BrowserWorkspace.h"
#include "MainWindow.h"
#include "Profile.h"
#include "ProfileManager.h"
#include "ProfileManagementDialog.h"
#include "Provider.h"
#include "ProviderRegistry.h"
#include "SettingsManager.h"
#include "Sidebar.h"
#include "TabStore.h"
#include "Theme.h"

#include <QApplication>
#include <QComboBox>
#include <QDir>
#include <QElapsedTimer>
#include <QEventLoop>
#include <QFile>
#include <QImage>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QPainter>
#include <QPixmap>
#include <QScreen>
#include <QScrollArea>
#include <QScrollBar>
#include <QSet>
#include <QStandardPaths>
#include <QTimer>
#include <QToolButton>
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

enum class LoadOutcome { Loaded, Failed, Timeout };

LoadOutcome awaitTabLoad(MainWindow* window) {
  BrowserWorkspace* workspace = window->workspaceForTest();
  const bool settled = waitUntil(
      [window, workspace]() {
        BrowserTab* tab = window->currentTabForTest();
        if (!tab || workspace->tabCount() == 0) {
          return false;
        }
        const LifecycleState state = tab->lifecycleState();
        return state != LifecycleState::Loading && state != LifecycleState::Unloaded;
      },
      kLoadTimeoutMs);
  if (!settled) {
    return LoadOutcome::Timeout;
  }

  BrowserTab* tab = window->currentTabForTest();
  if (!tab || tab->lifecycleState() != LifecycleState::Active) {
    return LoadOutcome::Failed;
  }

  QWebEngineView* view = tab->view();
  if (!view) {
    return LoadOutcome::Failed;
  }
  return waitUntil([view]() { return hasRenderedContent(view->grab()); }, 10000)
             ? LoadOutcome::Loaded
             : LoadOutcome::Failed;
}

BrowserTab* waitForLoad(MainWindow* window) {
  const LoadOutcome outcome = awaitTabLoad(window);
  require(outcome != LoadOutcome::Timeout,
          "Provider page did not leave the Loading/Unloaded state before timeout");
  BrowserTab* tab = window->currentTabForTest();
  require(tab != nullptr, "No active tab after navigation");
  require(outcome == LoadOutcome::Loaded,
          QString("Navigation did not succeed: %1")
              .arg(lifecycleStateToString(tab->lifecycleState())));
  return tab;
}

void openProviderAndWait(MainWindow* window, const QString& providerId) {
  BrowserWorkspace* workspace = window->workspaceForTest();

  for (int attempt = 1; attempt <= 3; ++attempt) {
    const int before = workspace->tabCount();
    window->openProviderForTest(providerId);
    require(workspace->tabCount() == before + 1,
            QString("Provider tab was not created: %1").arg(providerId));

    const LoadOutcome outcome = awaitTabLoad(window);
    BrowserTab* tab = window->currentTabForTest();
    if (outcome == LoadOutcome::Loaded && tab && tab->providerId() == providerId) {
      return;
    }

    std::fprintf(stderr, "Retrying provider %s after load attempt %d failed\n",
                 qPrintable(providerId), attempt);
    const QString tabId = workspace->currentTabId();
    if (!tabId.isEmpty()) {
      QMetaObject::invokeMethod(window, "onTabClosed", Qt::DirectConnection,
                                Q_ARG(QString, tabId));
      QApplication::processEvents();
    }
    QEventLoop loop;
    QTimer::singleShot(1000, &loop, &QEventLoop::quit);
    loop.exec();
  }

  fail(QString("Provider page did not load after three attempts: %1").arg(providerId));
}

void verifyProviderStrip(MainWindow* window, bool scrollToEnd) {
  Sidebar* sidebar = window->sidebarForTest();
  QWidget* strip = sidebar->findChild<QWidget*>("providerStrip");
  require(strip != nullptr, "Provider strip is missing");

  const QList<QToolButton*> buttons = strip->findChildren<QToolButton*>("providerStripButton");
  require(buttons.size() == 9, QString("Provider strip has %1 buttons, expected 9")
                                   .arg(buttons.size()));

  const int top = buttons.first()->geometry().top();
  for (QToolButton* button : buttons) {
    require(button->geometry().top() == top, "Provider buttons are not arranged horizontally");
    require(!button->toolTip().isEmpty(), "Provider button has no tooltip");
    require(!button->icon().isNull(), "Provider button has no icon");
  }

  QScrollArea* area = sidebar->findChild<QScrollArea*>("providerScrollArea");
  require(area != nullptr, "Provider scroll area is missing");
  require(area->horizontalScrollBarPolicy() == Qt::ScrollBarAlwaysOff,
          "Provider strip must not show a permanent scrollbar");
  QScrollBar* bar = area->horizontalScrollBar();
  require(bar->maximum() > 0, "Provider strip should be horizontally scrollable");

  if (scrollToEnd) {
    bar->setValue(bar->maximum());
    QApplication::processEvents();
    require(bar->value() == bar->maximum(), "Provider strip did not scroll to the end");
  }

  QToolButton* last = buttons.last();
  const QRect lastRect(last->mapTo(area->viewport(), QPoint(0, 0)), last->size());
  if (scrollToEnd) {
    require(area->viewport()->rect().contains(lastRect),
            "Last provider is not reachable when scrolled to the end");
  }

  QToolButton* first = buttons.first();
  const QRect firstRect(first->mapTo(area->viewport(), QPoint(0, 0)), first->size());
  if (!scrollToEnd) {
    require(area->viewport()->rect().intersects(firstRect),
            "First provider is not visible at the start of the strip");
  }
}

QJsonObject sessionMetaObject() {
  QJsonArray tabs;
  for (const TabMetadata& tab : TabStore::load()) {
    QJsonObject entry;
    entry["providerId"] = tab.providerId();
    entry["profileId"] = tab.profileId();
    entry["url"] = tab.url().toString();
    entry["title"] = tab.title();
    tabs.append(entry);
  }

  QJsonObject root;
  root["version"] = TabStore::kSessionVersion;
  root["currentIndex"] = TabStore::currentIndex();
  root["tabs"] = tabs;
  return root;
}

void writeSessionMeta() {
  QDir dir(g_outputDir);
  if (!dir.exists() && !dir.mkpath(".")) {
    fail(QString("Cannot create output directory: %1").arg(g_outputDir));
  }
  QFile file(dir.filePath("session-meta.json"));
  require(file.open(QIODevice::WriteOnly | QIODevice::Truncate),
          "Cannot write session metadata for restore verification");
  file.write(QJsonDocument(sessionMetaObject()).toJson());
  file.close();
  std::printf("Wrote %s\n", qPrintable(file.fileName()));
}

void clearTestProfiles() {
  ProfileManager* manager = ProfileManager::instance();
  const QList<Profile*> profiles = manager->profiles();
  for (Profile* profile : profiles) {
    while (profile->refCount() > 0) {
      profile->deref();
    }
    manager->deleteProfile(profile);
  }
}

void verifyRestoredSession(MainWindow* window) {
  QFile file(QDir(g_outputDir).filePath("session-meta.json"));
  require(file.open(QIODevice::ReadOnly), "Restore state requires session-meta.json");
  const QJsonObject root = QJsonDocument::fromJson(file.readAll()).object();
  const QJsonArray expectedTabs = root["tabs"].toArray();
  require(!expectedTabs.isEmpty(), "Restore metadata contains no tabs");

  BrowserWorkspace* workspace = window->workspaceForTest();
  require(workspace->tabCount() == expectedTabs.size(),
          QString("Restored tab count %1 does not match saved count %2")
              .arg(workspace->tabCount())
              .arg(expectedTabs.size()));

  const QStringList ids = workspace->tabIds();
  for (int i = 0; i < expectedTabs.size(); ++i) {
    const QJsonObject expected = expectedTabs.at(i).toObject();
    BrowserTab* tab = workspace->tabById(ids.at(i));
    require(tab != nullptr, "Restored workspace returned a null tab");
    require(tab->providerId() == expected["providerId"].toString(),
            QString("Restored tab %1 provider mismatch").arg(i));
    require(tab->profileId() == expected["profileId"].toString(),
            QString("Restored tab %1 profile mismatch").arg(i));
    require(tab->url() == QUrl(expected["url"].toString()),
            QString("Restored tab %1 URL mismatch: %2 != %3")
                .arg(i)
                .arg(tab->url().toString(), expected["url"].toString()));
  }

  const int savedIndex = root["currentIndex"].toInt();
  require(workspace->tabIndex(workspace->currentTabId()) == savedIndex,
          "Restored selected tab index does not match the saved index");

  BrowserTab* current = window->currentTabForTest();
  require(current != nullptr, "Restored session has no active tab");
  require(window->sidebarForTest()->selectedProvider() == current->providerId(),
          "Sidebar provider selection was not restored");

  for (const QString& tabId : ids) {
    if (tabId == workspace->currentTabId()) {
      continue;
    }
    BrowserTab* tab = workspace->tabById(tabId);
    require(tab->lifecycleState() == LifecycleState::Unloaded,
            QString("Restored inactive tab %1 is not Unloaded").arg(tabId));
    require(tab->page() == nullptr,
            QString("Restored inactive tab %1 created a WebEngine page").arg(tabId));
  }

  verifyProviderStrip(window, false);
  waitForLoad(window);
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
      verifyProviderStrip(window, false);
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

    case 6:
      openProviderAndWait(window, "deepseek");
      verifyProviderStrip(window, true);
      break;

    case 7:
      verifyRestoredSession(window);
      require(!sidebar->isCollapsed(), "Restored session capture expects an expanded sidebar");
      break;

    default:
      fail(QString("Unknown visual QA state index: %1").arg(state));
  }
}

}  // namespace

int main(int argc, char* argv[]) {
  QApplication app(argc, argv);
  QStandardPaths::setTestModeEnabled(true);
  app.setApplicationName("AI Workspace Visual QA");

  if (argc >= 2 && QString(argv[1]) == "prepare") {
    if (argc < 3) {
      std::fprintf(stderr, "Usage: %s prepare <output-dir>\n", argv[0]);
      return 1;
    }

    g_outputDir = QString::fromUtf8(argv[2]);
    SettingsManager::instance()->setTheme("light");
    SettingsManager::instance()->reset("sidebar/collapsed");
    SettingsManager::instance()->reset("sidebar/geometry");
    TabStore::clear();
    clearTestProfiles();
    Theme::applyTheme("light");

    auto* window = new MainWindow();
    window->resize(kWindowWidth, kWindowHeight);
    window->show();
    require(waitUntil([window]() { return isExposed(window); }, 10000),
            "Main window did not become visible while preparing the session");

    openProviderAndWait(window, "deepseek");
    openProviderAndWait(window, "qwen");
    openProviderAndWait(window, "chatgpt");

    const QStringList ids = window->workspaceForTest()->tabIds();
    require(ids.size() == 3, "Prepared session does not have three tabs");
    window->selectTabForTest(ids.first());
    QApplication::processEvents();

    window->close();
    QApplication::processEvents();

    require(TabStore::load().size() == 3, "Prepared session was not saved");
    writeSessionMeta();

    delete window;
    app.processEvents();
    std::fflush(stdout);
    std::fflush(stderr);
    std::_Exit(0);
  }

  if (argc < 4) {
    std::fprintf(stderr,
                 "Usage: %s <state 0-7> <light|dark> <output-dir>\n"
                 "       %s prepare <output-dir>\n"
                 "States: 0 initial, 1 active, 2 multiple, 3 collapsed, 4 empty,\n"
                 "        5 accounts, 6 provider-strip-end, 7 restored-session\n",
                 argv[0], argv[0]);
    return 1;
  }

  const int state = std::atoi(argv[1]);
  g_dark = QString(argv[2]) == "dark";
  g_outputDir = QString::fromUtf8(argv[3]);
  const bool restoring = state == 7;

  const QString theme = g_dark ? "dark" : "light";
  SettingsManager::instance()->setTheme(theme);
  SettingsManager::instance()->reset("sidebar/collapsed");
  SettingsManager::instance()->reset("sidebar/geometry");
  if (!restoring) {
    TabStore::clear();
  } else {
    require(!TabStore::load().isEmpty(), "No saved session to restore");
  }
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
    QPixmap shot;
    bool captured = false;
    for (int attempt = 0; attempt < 5 && !captured; ++attempt) {
      window->resize(kWindowWidth, kWindowHeight);
      require(waitUntil(
                  [window]() { return window->size() == QSize(kWindowWidth, kWindowHeight); },
                  10000),
              "Main window changed size before capture");
      QApplication::processEvents();
      shot = captureWindow(window);
      captured = !shot.isNull() && shot.size() == QSize(kWindowWidth, kWindowHeight) &&
                 hasRenderedContent(shot);
      if (!captured) {
        QEventLoop loop;
        QTimer::singleShot(200, &loop, &QEventLoop::quit);
        loop.exec();
      }
    }
    require(captured, "Main window capture is blank or has an unexpected size");

    QString name;
    switch (state) {
      case 0: name = "01-initial"; break;
      case 1: name = "02-active-workspace"; break;
      case 2: name = "03-multiple-tabs"; break;
      case 3: name = "04-sidebar-collapsed"; break;
      case 4: name = "05-empty-workspace"; break;
      case 6: name = "07-provider-strip-end"; break;
      case 7: name = "08-restored-session"; break;
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
