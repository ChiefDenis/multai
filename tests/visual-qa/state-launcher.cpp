#include "MainWindow.h"
#include "BrowserWorkspace.h"
#include "SettingsManager.h"
#include "Theme.h"
#include "ProfileManagementDialog.h"
#include <QApplication>
#include <QTimer>
#include <QScreen>
#include <QPixmap>
#include <QDebug>
#include <QDir>
#include <QGuiApplication>
#include <QWindow>
#include <QFile>
#include <iostream>

static int g_state = 0;
static bool g_dark = false;
static bool g_capture_only = false;

int main(int argc, char* argv[]) {
  QApplication app(argc, argv);
  if (argc > 1) g_state = std::stoi(argv[1]);
  if (argc > 2) g_dark = (std::string(argv[2]) == "dark");
  if (argc > 3) g_capture_only = (std::string(argv[3]) == "capture");

  SettingsManager::instance()->setTheme(g_dark ? "dark" : "light");
  Theme::applyTheme(g_dark ? "dark" : "light");

  MainWindow window;
  window.resize(1366, 768);
  window.show();

  // Process initial events to show the window
  app.processEvents();

  QTimer::singleShot(800, [&]() {
    // Set up the requested UI state
    switch (g_state) {
      case 0: // Initial, no tabs (empty workspace)
        break;
      case 1: { // One tab (DeepSeek) - active provider
        window.openProviderForTest("deepseek");
        break;
      }
      case 2: { // Multiple tabs (DeepSeek + Qwen + Gemini)
        window.openProviderForTest("deepseek");
        QTimer::singleShot(400, [&]() { window.openProviderForTest("qwen"); });
        QTimer::singleShot(800, [&]() { window.openProviderForTest("gemini"); });
        break;
      }
      case 3: { // Collapsed sidebar with a tab
        window.openProviderForTest("deepseek");
        QTimer::singleShot(400, [&]() { window.collapseSidebarForTest(); });
        break;
      }
      case 4: // Empty workspace (no tabs) - same visual as state 0
        break;
      case 5: { // Account management dialog
        auto* dialog = new ProfileManagementDialog(&window);
        dialog->setAttribute(Qt::WA_DeleteOnClose);
        dialog->show();
        dialog->raise();
        dialog->activateWindow();
        break;
      }
    }
    if (g_dark) {
      window.setThemeForTest("dark");
    }
  });

  // Capture after setup (WebEngine needs time to initialize pages)
  QTimer::singleShot(3000, [&]() {
    QPixmap pixmap(window.size());
    window.render(&pixmap);
    QString outputPath = QString("./capture-%1-%2.png")
        .arg(QString::number(g_state), g_dark ? "dark" : "light");
    QDir dir("./");
    dir.mkpath(".");
    pixmap.save(outputPath, "PNG");
    std::cout << "Captured: " << outputPath.toStdString() << std::endl;
    app.quit();
  });

  return app.exec();
}
