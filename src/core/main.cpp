#include "Application.h"
#include "MainWindow.h"
#include <QCommandLineParser>
#include <QCommandLineOption>
#include <cstdio>

int main(int argc, char* argv[]) {
  Application app(argc, argv);

  QCommandLineParser parser;
  parser.setApplicationDescription("Multai - Multi-provider AI desktop application");
  parser.addHelpOption();
  parser.addVersionOption();
  parser.process(app);

  if (parser.isSet("version")) {
    std::printf("Multai %s\n", qPrintable(app.applicationVersion()));
    return 0;
  }

  MainWindow* mainWindow = new MainWindow();
  app.setMainWindow(mainWindow);
  mainWindow->show();

  return app.exec();
}
