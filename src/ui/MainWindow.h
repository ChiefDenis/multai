#pragma once

#include <QMainWindow>

#include "lifecycle/TabMetadata.h"
#include "core/SettingsManager.h"
#include "ui/Sidebar.h"

class BrowserTab;
class BrowserWorkspace;
class QSplitter;
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
  void onProviderAddAccount(const QString& providerId);
  void onProviderManageAccounts(const QString& providerId);
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
  void setupCentralWidget();
  void setupStatusBar();
  void applyTheme();
  void restoreSidebarState();
  void saveSidebarState();
  void restoreTabs();
  void saveSession();

  BrowserTab* createBrowserTab(const QString& providerId, const QString& profileId,
                               const QUrl& url, const QString& title);

  QString nextTabId();

  void updateStatusDisplay(BrowserTab* tab);
  QString tabDisplayTitle(BrowserTab* tab) const;

  Sidebar* m_sidebar;
  BrowserWorkspace* m_workspace;
  QSplitter* m_splitter;
  QMenu* m_fileMenu;
  QMenu* m_helpMenu;
  QLabel* m_statusTabInfoLabel;
  QLabel* m_statusBadgeLabel;
  int m_tabCounter;
  bool m_restoringSession;
  BrowserTab* m_currentTab;

public:
  // Visual QA test hooks
  Sidebar* sidebarForTest() const { return m_sidebar; }
  BrowserWorkspace* workspaceForTest() const { return m_workspace; }
  QLabel* statusTabInfoLabelForTest() const { return m_statusTabInfoLabel; }
  QLabel* statusBadgeLabelForTest() const { return m_statusBadgeLabel; }
  BrowserTab* currentTabForTest() const { return m_currentTab; }
  void updateStatusDisplayForTest(BrowserTab* tab) { updateStatusDisplay(tab); }
  QString tabDisplayTitleForTest(BrowserTab* tab) const { return tabDisplayTitle(tab); }
  void collapseSidebarForTest() { m_sidebar->setCollapsed(true); }
  void expandSidebarForTest() { m_sidebar->setCollapsed(false); }
  void setThemeForTest(const QString& theme) { SettingsManager::instance()->setTheme(theme); applyTheme(); }
  void openProviderForTest(const QString& providerId) { onProviderSelected(providerId); }
  void saveSessionForTest() { saveSession(); }
  void selectTabForTest(const QString& tabId) { onTabSelected(tabId); }
};
