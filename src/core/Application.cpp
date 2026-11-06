#include "Application.h"
#include "MainWindow.h"

Application::Application(int& argc, char** argv)
  : QApplication(argc, argv)
  , m_mainWindow(nullptr) {
  setApplicationName("AI Workspace");
  setApplicationVersion("0.3.0");
  setOrganizationName("ai-workspace");
  setOrganizationDomain("ai-workspace.local");
}
