#include "Application.h"
#include "MainWindow.h"

#include <QGuiApplication>
#include <QIcon>
#include "version.h"

Application::Application(int& argc, char** argv)
  : QApplication(argc, argv)
  , m_mainWindow(nullptr) {
  setApplicationName("AI Workspace");
  setApplicationVersion(AI_WORKSPACE_VERSION);
  setOrganizationName("ai-workspace");
  setOrganizationDomain("ai-workspace.local");
  QGuiApplication::setDesktopFileName("ai-workspace");
  setWindowIcon(QIcon(":/icons/hicolor/256x256/apps/ai-workspace.png"));
}
