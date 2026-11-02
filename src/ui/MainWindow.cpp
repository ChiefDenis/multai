#include "MainWindow.h"
#include "LifecycleManager.h"
#include "ProfileManagementDialog.h"
#include "SettingsDialog.h"
#include "SettingsManager.h"
#include "TabWidget.h"
#include <QAction>
#include <QApplication>
#include <QCloseEvent>
#include <QMenuBar>
#include <QMessageBox>
#include <QStyle>
#include <QToolBar>

MainWindow::MainWindow(QWidget* parent)
  : QMainWindow(parent)
  , m_tabWidget(nullptr)
  , m_toolBar(nullptr)
  , m_fileMenu(nullptr)
  , m_helpMenu(nullptr) {
  setWindowTitle("AI Workspace");
  resize(1200, 800);

  setupMenuBar();
  setupToolBar();
  setupCentralWidget();
  applyTheme();

  SettingsManager* settings = SettingsManager::instance();
  restoreGeometry(settings->windowGeometry());
  restoreState(settings->windowState());

  m_tabWidget->restoreTabs();
}

void MainWindow::setupMenuBar() {
  m_fileMenu = menuBar()->addMenu("&File");

  QAction* newTabAction = m_fileMenu->addAction("&New Tab");
  newTabAction->setShortcut(QKeySequence::New);
  newTabAction->setStatusTip("Create a new tab");
  newTabAction->setToolTip("Create a new tab (Ctrl+N)");
  connect(newTabAction, &QAction::triggered, this, &MainWindow::onNewTab);

  m_fileMenu->addSeparator();

  QAction* profilesAction = m_fileMenu->addAction("&Profiles...");
  profilesAction->setStatusTip("Manage provider accounts");
  profilesAction->setToolTip("Create, rename and delete provider accounts");
  connect(profilesAction, &QAction::triggered, this, &MainWindow::onProfiles);

  QAction* settingsAction = m_fileMenu->addAction("&Settings");
  settingsAction->setShortcut(QKeySequence::Preferences);
  settingsAction->setStatusTip("Open settings");
  settingsAction->setToolTip("Open settings (Ctrl+,)");
  connect(settingsAction, &QAction::triggered, this, &MainWindow::onSettings);

  m_fileMenu->addSeparator();

  QAction* quitAction = m_fileMenu->addAction("&Quit");
  quitAction->setShortcut(QKeySequence::Quit);
  quitAction->setStatusTip("Quit application");
  quitAction->setToolTip("Quit application (Ctrl+Q)");
  connect(quitAction, &QAction::triggered, this, &MainWindow::onQuit);

  m_helpMenu = menuBar()->addMenu("&Help");

  QAction* aboutAction = m_helpMenu->addAction("&About");
  aboutAction->setStatusTip("About AI Workspace");
  connect(aboutAction, &QAction::triggered, this, &MainWindow::onAbout);
}

void MainWindow::setupToolBar() {
  m_toolBar = addToolBar("Main");
  m_toolBar->setMovable(false);
  m_toolBar->setFloatable(false);

  QAction* newTabAction = m_toolBar->addAction(style()->standardIcon(QStyle::SP_FileIcon), "New Tab");
  newTabAction->setShortcut(QKeySequence::New);
  newTabAction->setStatusTip("Create a new tab");
  newTabAction->setToolTip("Create a new tab");
  connect(newTabAction, &QAction::triggered, this, &MainWindow::onNewTab);
}

void MainWindow::setupCentralWidget() {
  m_tabWidget = new TabWidget(this);
  setCentralWidget(m_tabWidget);
}

void MainWindow::applyTheme() {
  SettingsManager* settings = SettingsManager::instance();
  QString theme = settings->theme();

  if (theme == "dark") {
    setStyleSheet(R"(
      QMainWindow { background-color: #1e1e1e; }
      QMenuBar { background-color: #2d2d2d; color: #ffffff; }
      QMenuBar::item { background-color: transparent; padding: 4px 8px; }
      QMenuBar::item:selected { background-color: #3d3d3d; }
      QToolBar { background-color: #2d2d2d; color: #ffffff; border: none; }
      QToolBar::separator { background-color: #3d3d3d; width: 1px; }
      QStackedWidget { background-color: #1e1e1e; }
      QLabel { color: #ffffff; }
      QPushButton { background-color: #3d3d3d; color: #ffffff; border: none; padding: 8px 16px; border-radius: 6px; }
      QPushButton:hover { background-color: #4d4d4d; }
      QPushButton:pressed { background-color: #2d2d2d; }
    )");
  } else if (theme == "light") {
    setStyleSheet(R"(
      QMainWindow { background-color: #ffffff; }
      QMenuBar { background-color: #f6f6f6; color: #000000; }
      QMenuBar::item { background-color: transparent; padding: 4px 8px; }
      QMenuBar::item:selected { background-color: #e5e5e5; }
      QToolBar { background-color: #f6f6f6; color: #000000; border: none; }
      QToolBar::separator { background-color: #d0d0d0; width: 1px; }
      QStackedWidget { background-color: #ffffff; }
      QLabel { color: #000000; }
      QPushButton { background-color: #e5e5e5; color: #000000; border: none; padding: 8px 16px; border-radius: 6px; }
      QPushButton:hover { background-color: #d5d5d5; }
      QPushButton:pressed { background-color: #c5c5c5; }
    )");
  } else {
    setStyleSheet("");
  }
}

void MainWindow::closeEvent(QCloseEvent* event) {
  SettingsManager* settings = SettingsManager::instance();
  if (m_tabWidget) {
    m_tabWidget->saveTabs();
  }
  settings->setWindowGeometry(saveGeometry());
  settings->setWindowState(saveState());
  QMainWindow::closeEvent(event);
}

void MainWindow::onNewTab() {
  if (m_tabWidget) {
    m_tabWidget->showProviderSelection();
  }
}

void MainWindow::onProfiles() {
  ProfileManagementDialog dialog(this);
  dialog.exec();
}

void MainWindow::onSettings() {
  SettingsDialog dialog(this);
  if (dialog.exec() == QDialog::Accepted) {
    applyTheme();
    LifecycleManager::instance()->applySettings();
  }
}

void MainWindow::onAbout() {
  QMessageBox::about(this, "About AI Workspace",
                     QString("<b>AI Workspace %1</b><br><br>"
                             "A resource-conscious, multi-provider AI desktop application.<br>"
                             "Provider web interfaces are embedded with Qt WebEngine.")
                         .arg(QApplication::applicationVersion()));
}

void MainWindow::onQuit() {
  close();
}
