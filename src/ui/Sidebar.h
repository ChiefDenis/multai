#pragma once

#include <QIcon>
#include <QList>
#include <QMap>
#include <QPoint>
#include <QWidget>

class BrowserTab;
class QLabel;
class QListWidget;
class QListWidgetItem;
class QResizeEvent;
class QToolButton;
class QMenu;

#include "lifecycle/TabMetadata.h"

class Sidebar : public QWidget {
  Q_OBJECT
public:
  explicit Sidebar(QWidget* parent = nullptr);
  ~Sidebar() override = default;

  void addProviderRow(const QString& providerId, const QString& displayName,
                      const QIcon& icon);
  void addTabRow(const QString& tabId, const QString& providerId, const QString& title,
                 const QIcon& icon, LifecycleState state, bool hasError);
  void updateTabRow(const QString& tabId, const QString& title, LifecycleState state,
                    bool hasError);
  void removeTabRow(const QString& tabId);
  void selectTabRow(const QString& tabId);
  void clearTabRows();

  int sidebarWidth() const;
  void setSidebarWidth(int width);
  bool isCollapsed() const;
  void setCollapsed(bool collapsed);
  void updateTheme(const QString& theme);

  // Test accessors
  QListWidget* providerListForTest() const { return m_providerList; }
  QListWidget* tabListForTest() const { return m_tabList; }
  QWidget* collapsedRailForTest() const { return m_collapsedRail; }
  QToolButton* collapsedTabsButtonForTest() const { return m_collapsedTabsButton; }
  QToolButton* collapseButtonForTest() const { return m_collapseButton; }
  QToolButton* settingsButtonForTest() const { return m_settingsButton; }

signals:
  void providerSelected(const QString& providerId);
  void providerAddAccountRequested(const QString& providerId);
  void providerManageAccountsRequested(const QString& providerId);
  void tabSelected(const QString& tabId);
  void tabClosed(const QString& tabId);
  void tabActionRequested(const QString& tabId, const QString& action);
  void settingsRequested();
  void sidebarWidthChanged(int width);
  void sidebarCollapsedChanged(bool collapsed);

private:
  void setupUi();
  void setupProviderSection();
  void setupTabsSection();
  void setupCollapsedRail();
  void setupBottomSection();
  void updateCollapsedTabsButton();
  void updateProviderListHeight();
  void showCollapsedTabsMenu();

protected:
  void resizeEvent(QResizeEvent* event) override;

private:
  QListWidgetItem* createProviderItem(const QString& providerId,
                                      const QString& displayName, const QIcon& icon);
  QListWidgetItem* createTabItem(const QString& tabId, const QString& providerId,
                                 const QString& title, const QIcon& icon,
                                 LifecycleState state, bool hasError);
  void updateTabItemWidget(QListWidgetItem* item, const QString& title,
                           LifecycleState state, bool hasError);
  void onItemClicked(QListWidgetItem* item);
  void showTabContextMenu(const QPoint& pos);
  void showProviderContextMenu(const QPoint& pos);
  void executeTabAction(const QString& tabId, const QString& action);

  QWidget* m_expandedContainer;
  QWidget* m_collapsedRail;
  QLabel* m_headerAppIcon;
  QLabel* m_headerAppLabel;
  QToolButton* m_collapsedTabsButton;
  QListWidget* m_providerList;
  QListWidget* m_tabList;
  QToolButton* m_collapseButton;
  QToolButton* m_settingsButton;

  QMap<QString, QListWidgetItem*> m_tabItems;
  QMap<QString, QString> m_tabIdToProviderId;
  QMap<QString, QToolButton*> m_collapsedProviderButtons;
  int m_sidebarWidth;
  bool m_collapsed;
};
