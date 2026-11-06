#include "MainWindow.h"
#include "SettingsManager.h"
#include "Theme.h"
#include "ProfileManagementDialog.h"
#include "BrowserWorkspace.h"
#include "BrowserTab.h"

#include <QApplication>
#include <QPixmap>
#include <QTimer>
#include <QDebug>
#include <QDir>

static MainWindow* window = nullptr;
static QApplication* appInstance = nullptr;
static ProfileManagementDialog* g_dialog = nullptr;

void captureState(const QString& stateName, const QString& themeName) {
  QPixmap pixmap(window->size());
  window->render(&pixmap);
  QString filename = QString("artifacts/visual-qa/%1-%2.png").arg(stateName, themeName);
  pixmap.save(filename, "PNG");
  qDebug() << "Saved:" << filename << "size:" << pixmap.size();
}

void nextStep() {
  static int step = 0;
  step++;

  switch (step) {
    case 1: {
      qDebug() << "Step 1: Initial state (light, expanded)";
      captureState("01-initial-expanded", "light");
      window->openProviderForTest("deepseek");
      QTimer::singleShot(800, nextStep);
      break;
    }
    case 2: {
      qDebug() << "Step 2: Active workspace (light)";
      captureState("02-active-workspace", "light");
      window->openProviderForTest("qwen");
      QTimer::singleShot(800, nextStep);
      break;
    }
    case 3: {
      qDebug() << "Step 3: Multiple tabs (light)";
      captureState("03-multiple-tabs", "light");
      window->collapseSidebarForTest();
      QTimer::singleShot(300, nextStep);
      break;
    }
    case 4: {
      qDebug() << "Step 4: Collapsed sidebar (light)";
      captureState("04-sidebar-collapsed", "light");
      window->expandSidebarForTest();
      QTimer::singleShot(300, nextStep);
      break;
    }
    case 5: {
      qDebug() << "Step 5: Closing all tabs for empty state";
      auto* ws = window->workspaceForTest();
      while (ws->tabCount() > 0) {
        auto* tab = ws->currentTab();
        if (tab) {
          ws->removeTab(tab, tab->property("tabId").toString());
        }
      }
      QTimer::singleShot(300, nextStep);
      break;
    }
    case 6: {
      qDebug() << "Step 6: Empty workspace (light)";
      captureState("08-empty-workspace", "light");
      window->setThemeForTest("dark");
      QTimer::singleShot(500, nextStep);
      break;
    }
    case 7: {
      qDebug() << "Step 7: Empty workspace (dark)";
      captureState("09-empty-workspace-dark", "dark");
      window->openProviderForTest("claude");
      QTimer::singleShot(800, nextStep);
      break;
    }
    case 8: {
      qDebug() << "Step 8: Active workspace (dark)";
      captureState("10-active-workspace-dark", "dark");
      window->collapseSidebarForTest();
      QTimer::singleShot(300, nextStep);
      break;
    }
    case 9: {
      qDebug() << "Step 9: Collapsed sidebar (dark)";
      captureState("11-sidebar-collapsed-dark", "dark");
      window->expandSidebarForTest();
      QTimer::singleShot(300, nextStep);
      break;
    }
    case 10: {
      qDebug() << "Step 10: Account management dialog";
      g_dialog = new ProfileManagementDialog(window);
      g_dialog->setAttribute(Qt::WA_DeleteOnClose);
      g_dialog->show();
      QTimer::singleShot(800, []() {
        captureState("12-account-management", "light");
        g_dialog->close();
        QTimer::singleShot(500, nextStep);
      });
      break;
    }
    case 11: {
      qDebug() << "Step 11: System theme";
      window->setThemeForTest("system");
      QTimer::singleShot(500, nextStep);
      break;
    }
    case 12: {
      qDebug() << "Step 12: System theme captured";
      captureState("13-system-theme", "system");
      QTimer::singleShot(500, []() { appInstance->quit(); });
      break;
    }
    default:
      appInstance->quit();
      break;
  }
}

int main(int argc, char* argv[]) {
  QApplication app(argc, argv);
  appInstance = &app;

  QDir outputDir("artifacts/visual-qa");
  if (!outputDir.exists()) {
    outputDir.mkpath(".");
  }

  SettingsManager::instance()->setTheme("light");
  Theme::applyTheme("light");

  window = new MainWindow();
  window->resize(1366, 768);
  window->setFixedSize(1366, 768);  // Fixed size for consistent screenshots
  window->show();

  QTimer::singleShot(1000, nextStep);

  return app.exec();
}
