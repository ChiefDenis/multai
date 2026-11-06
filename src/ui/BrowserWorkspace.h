#pragma once

#include <QList>
#include <QStackedWidget>
#include <QWidget>

class BrowserTab;
class QLabel;
class QPushButton;
class QVBoxLayout;
class QGridLayout;
class QFrame;

class BrowserWorkspace : public QStackedWidget {
  Q_OBJECT
public:
  explicit BrowserWorkspace(QWidget* parent = nullptr);
  ~BrowserWorkspace() override = default;

  void addTab(BrowserTab* tab, const QString& tabId);
  void removeTab(BrowserTab* tab, const QString& tabId);
  void activateTab(const QString& tabId);
  BrowserTab* currentTab() const;
  QString currentTabId() const;
  int tabCount() const;
  bool hasTabs() const;
  BrowserTab* tabById(const QString& tabId) const;

  void showEmptyState();
  void hideEmptyState();

signals:
  void tabActivated(BrowserTab* tab, const QString& tabId);
  void tabAdded(BrowserTab* tab, const QString& tabId);
  void tabRemoved(const QString& tabId);
  void emptyStateProviderRequested(const QString& providerId);

private:
  void setupEmptyState();
  void updateEmptyState();

  QWidget* m_emptyStateWidget;
  QMap<QString, BrowserTab*> m_tabs;
  QString m_currentTabId;
};
