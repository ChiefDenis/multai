#include "MainWindow.h"
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

  QTimer::singleShot(500, [&]() {
    switch (g_state) {
      case 0: // Initial, no tabs
        break;
      case 1: // One tab (DeepSeek)
        window.openProviderForTest("deepseek");
        break;
      case 2: // Two tabs (DeepSeek + Qwen)
        window.openProviderForTest("deepseek");
        QTimer::singleShot(300, [&]() { window.openProviderForTest("qwen"); });
        break;
      case 3: // Collapsed sidebar
        window.openProviderForTest("deepseek");
        QTimer::singleShot(300, [&]() { window.collapseSidebarForTest(); });
        break;
      case 4: // Empty workspace (no tabs)
        break;
      case 5: // Account management dialog
      {
        auto* dialog = new ProfileManagementDialog(&window);
        dialog->setAttribute(Qt::WA_DeleteOnClose);
        dialog->show();
        break;
      }
    }
    if (g_dark) {
      window.setThemeForTest("dark");
    }
  });

  if (g_capture_only) {
    // Capture immediately and exit
    QTimer::singleShot(1500, [&]() {
      QScreen* screen = window.windowHandle()->screen();
      if (!screen) {
        screen = QGuiApplication::primaryScreen();
      }
      QPixmap pixmap = screen->grabWindow(window.winId());
      QString outputPath = QString("/tmp/qa-capture/%1-%2.png")
          .arg(QString::number(g_state), g_dark ? "dark" : "light");
      QDir dir("/tmp/qa-capture");
      dir.mkpath(".");
      pixmap.save(outputPath, "PNG");
      std::cout << "Captured: " << outputPath.toStdString() << std::endl;
      app.quit();
    });
  }

  return app.exec();
}
