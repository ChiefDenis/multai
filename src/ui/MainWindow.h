#pragma once

#include <QMainWindow>

#include "lifecycle/TabMetadata.h"

class BrowserTab;
class BrowserWorkspace;
class QSplitter;
class QToolBar;
class QMenu;
class QLabel;

class Sidebar;

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

  void onProviderSelected(const QString& providerId);
  void onTabSelected(const QString& tabId);
  void onTabClosed(const QString& tabId);
  void onTabActionRequested(const QString& tabId, const QString& action);
  void onSettingsRequested();
  void onSidebarWidthChanged(int width);
  void onSidebarCollapsedChanged(bool collapsed);

  void onTabActivated(BrowserTab* tab, const QString& tabId);
  void onTabAdded(BrowserTab* tab, const QString& tabId);
  void onTabRemoved(const QString& tabId);

private:
  void setupMenuBar();
  void setupToolBar();
  void setupCentralWidget();
  void setupStatusBar();
  void applyTheme();
  void restoreSidebarState();
  void saveSidebarState();
  void restoreTabs();

  BrowserTab* createBrowserTab(const QString& providerId, const QString& profileId,
                               const QUrl& url, const QString& title);

  QString nextTabId();

  void updateStatusDisplay(BrowserTab* tab);

  Sidebar* m_sidebar;
  BrowserWorkspace* m_workspace;
  QSplitter* m_splitter;
  QToolBar* m_toolBar;
  QMenu* m_fileMenu;
  QMenu* m_helpMenu;
  QLabel* m_statusTabInfoLabel;
  QLabel* m_statusBadgeLabel;
  int m_tabCounter;
  BrowserTab* m_currentTab;
};
