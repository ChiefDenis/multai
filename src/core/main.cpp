#include "Application.h"
#include <QCommandLineParser>
#include <QCommandLineOption>

int main(int argc, char* argv[]) {
  Application app(argc, argv);

  QCommandLineParser parser;
  parser.setApplicationDescription("AI Workspace - Multi-provider AI desktop application");
  parser.addHelpOption();
  parser.addVersionOption();
  parser.process(app);

  return app.exec();
}
