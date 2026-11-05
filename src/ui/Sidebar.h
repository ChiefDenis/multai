#pragma once

#include <QIcon>
#include <QList>
#include <QMap>
#include <QPoint>
#include <QWidget>

class BrowserTab;
class QListWidget;
class QListWidgetItem;
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

signals:
  void providerSelected(const QString& providerId);
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
  void setupBottomSection();
  QListWidgetItem* createProviderItem(const QString& providerId,
                                      const QString& displayName, const QIcon& icon);
  QListWidgetItem* createTabItem(const QString& tabId, const QString& providerId,
                                 const QString& title, const QIcon& icon,
                                 LifecycleState state, bool hasError);
  void updateTabItemWidget(QListWidgetItem* item, const QString& title,
                           LifecycleState state, bool hasError);
  void onItemClicked(QListWidgetItem* item);
  void showTabContextMenu(const QPoint& pos);
  void executeTabAction(const QString& tabId, const QString& action);

  QListWidget* m_providerList;
  QListWidget* m_tabList;
  QToolButton* m_collapseButton;
  QToolButton* m_settingsButton;

  QMap<QString, QListWidgetItem*> m_tabItems;
  QMap<QString, QString> m_tabIdToProviderId;
  int m_sidebarWidth;
  bool m_collapsed;
};
