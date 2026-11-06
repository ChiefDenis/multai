#pragma once

#include <QApplication>

class MainWindow;

class Application : public QApplication {
  Q_OBJECT
public:
  explicit Application(int& argc, char** argv);
  ~Application() override = default;

  MainWindow* mainWindow() const { return m_mainWindow; }
  void setMainWindow(MainWindow* mainWindow) { m_mainWindow = mainWindow; }

private:
  MainWindow* m_mainWindow;
};
