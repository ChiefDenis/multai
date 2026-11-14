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
class QScrollArea;
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
  void setSelectedProvider(const QString& providerId);
  QString selectedProvider() const { return m_selectedProviderId; }

// Test accessors
  QScrollArea* providerScrollAreaForTest() const { return m_providerScrollArea; }
  QWidget* providerStripForTest() const { return m_providerStrip; }
  QListWidget* tabListForTest() const { return m_tabList; }
  QWidget* collapsedRailForTest() const { return m_collapsedRail; }
  QScrollArea* collapsedTabScrollAreaForTest() const { return m_collapsedTabScrollArea; }
  QWidget* collapsedTabContainerForTest() const { return m_collapsedTabContainer; }
  const QMap<QString, QToolButton*>& collapsedTabButtonsForTest() const { return m_collapsedTabButtons; }
  QToolButton* collapseButtonForTest() const { return m_collapseButton; }
  QToolButton* settingsButtonForTest() const { return m_settingsButton; }
  QWidget* expandedContainerForTest() const { return m_expandedContainer; }

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
  void setupExpandedBottomSection();
  void applySelectedProvider(QToolButton* button, const QString& providerId);
  void rebuildCollapsedTabRail();

private:
  void updateTabItemWidget(QListWidgetItem* item, const QString& title,
                           LifecycleState state, bool hasError);
  void onItemClicked(QListWidgetItem* item);
  void showTabContextMenu(const QPoint& pos);
  void showProviderContextMenu(QToolButton* button, const QPoint& pos);
  void executeTabAction(const QString& tabId, const QString& action);

  QWidget* m_expandedContainer;
  QWidget* m_collapsedRail;
  QScrollArea* m_collapsedTabScrollArea;
  QWidget* m_collapsedTabContainer;
  QScrollArea* m_providerScrollArea;
  QWidget* m_providerStrip;
  QListWidget* m_tabList;
  QToolButton* m_collapseButton;
  QToolButton* m_settingsButton;

  QMap<QString, QListWidgetItem*> m_tabItems;
  QMap<QString, QString> m_tabIdToProviderId;
  QMap<QString, QToolButton*> m_providerButtons;
  QMap<QString, QToolButton*> m_collapsedTabButtons;
  QString m_selectedProviderId;
  int m_sidebarWidth;
  bool m_collapsed;
};
