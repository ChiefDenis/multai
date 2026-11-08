#include "MainWindow.h"
#include "SettingsManager.h"
#include "Theme.h"
#include "ProfileManagementDialog.h"
#include <QApplication>
#include <QTimer>
#include <iostream>

static int g_state = 0;
static bool g_dark = false;

int main(int argc, char* argv[]) {
  QApplication app(argc, argv);
  if (argc > 1) g_state = std::stoi(argv[1]);
  if (argc > 2) g_dark = (std::string(argv[2]) == "dark");

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
      case 3: // Collapsed sidebar (with one tab)
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

  // Stay alive - capture script will kill us after capturing
  return app.exec();
}
