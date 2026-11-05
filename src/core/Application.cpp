#include "Application.h"
#include "MainWindow.h"

Application::Application(int& argc, char** argv)
  : QApplication(argc, argv)
  , m_mainWindow(nullptr) {
  setApplicationName("AI Workspace");
  setApplicationVersion("0.2.2");
  setOrganizationName("ai-workspace");
  setOrganizationDomain("ai-workspace.local");

  m_mainWindow = new MainWindow();
  m_mainWindow->show();
}
