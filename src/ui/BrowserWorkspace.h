#pragma once

#include <QList>
#include <QStackedWidget>
#include <QStringList>
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

  void addTab(BrowserTab* tab, const QString& tabId, bool activate = true);
  void removeTab(BrowserTab* tab, const QString& tabId);
  void activateTab(const QString& tabId);
  BrowserTab* currentTab() const;
  QString currentTabId() const;
  int tabCount() const;
  bool hasTabs() const;
  BrowserTab* tabById(const QString& tabId) const;
  QStringList tabIds() const { return m_tabOrder; }
  int tabIndex(const QString& tabId) const { return m_tabOrder.indexOf(tabId); }
  bool moveTab(const QString& tabId, int newIndex);

  void showEmptyState();
  void hideEmptyState();
  void updateTheme(const QString& theme);

  QLabel* emptyStateFolderIconLabelForTest() const { return m_emptyStateFolderIconLabel; }

signals:
  void tabActivated(BrowserTab* tab, const QString& tabId);
  void tabAdded(BrowserTab* tab, const QString& tabId);
  void tabRemoved(const QString& tabId);
  void tabOrderChanged();
  void emptyStateProviderRequested(const QString& providerId);

private:
  void setupEmptyState();
  void updateEmptyState();

  QWidget* m_emptyStateWidget;
  QLabel* m_emptyStateFolderIconLabel;
  QMap<QString, BrowserTab*> m_tabs;
  QStringList m_tabOrder;
  QString m_currentTabId;
};
