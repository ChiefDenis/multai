#include "Application.h"
#include "MainWindow.h"

#include <QIcon>

Application::Application(int& argc, char** argv)
  : QApplication(argc, argv)
  , m_mainWindow(nullptr) {
  setApplicationName("AI Workspace");
  setApplicationVersion("0.4.0");
  setOrganizationName("ai-workspace");
  setOrganizationDomain("ai-workspace.local");
  setWindowIcon(QIcon(":/icons/hicolor/256x256/apps/ai-workspace.png"));
}
