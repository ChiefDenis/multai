#include "Application.h"
#include "AppIdentity.h"
#include "DataMigration.h"
#include "MainWindow.h"

#include <QGuiApplication>
#include <QIcon>
#include "version.h"

Application::Application(int& argc, char** argv)
  : QApplication(argc, argv)
  , m_mainWindow(nullptr) {
  DataMigration::run();

  setApplicationName(QString::fromLatin1(AppIdentity::kApplicationName));
  setApplicationDisplayName(QString::fromLatin1(AppIdentity::kApplicationDisplayName));
  setApplicationVersion(QString::fromLatin1(MULTAI_VERSION));
  setOrganizationName(QString::fromLatin1(AppIdentity::kOrganizationName));
  setOrganizationDomain(QString::fromLatin1(AppIdentity::kOrganizationDomain));
  QGuiApplication::setDesktopFileName(QString::fromLatin1(AppIdentity::kDesktopFileName));
  setWindowIcon(QIcon::fromTheme(AppIdentity::kIconName, QIcon(AppIdentity::kIconResource)));
}
