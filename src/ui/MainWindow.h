#pragma once

#include <QMainWindow>
#include <QStackedWidget>

class TabWidget;
class QToolBar;
class QMenu;

class MainWindow : public QMainWindow {
  Q_OBJECT
public:
  explicit MainWindow(QWidget* parent = nullptr);
  ~MainWindow() override = default;

protected:
  void closeEvent(QCloseEvent* event) override;

private slots:
  void onNewTab();
  void onProfiles();
  void onSettings();
  void onAbout();
  void onQuit();

private:
  void setupMenuBar();
  void setupToolBar();
  void setupCentralWidget();
  void applyTheme();

  TabWidget* m_tabWidget;
  QToolBar* m_toolBar;
  QMenu* m_fileMenu;
  QMenu* m_helpMenu;
};
