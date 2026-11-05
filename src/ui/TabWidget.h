#pragma once

#include <QList>
#include <QWidget>

class QLabel;
class QPushButton;
class QStackedWidget;
class QTabWidget;
class QVBoxLayout;
class BrowserTab;
class ProviderSelectionDialog;

class TabWidget : public QWidget {
  Q_OBJECT
public:
  explicit TabWidget(QWidget* parent = nullptr);
  ~TabWidget() override = default;

  void showProviderSelection();
  BrowserTab* currentTab() const;

  void restoreTabs();
  void saveTabs() const;

signals:
  void currentTabChanged(BrowserTab* tab);

private slots:
  void onProviderSelected(const QString& providerId);

private:
  void setupEmptyState();
  void setupTabs();
  BrowserTab* addBrowserTab(BrowserTab* tab, const QString& title);
  void removeBrowserTab(BrowserTab* tab);
  void updateTab(BrowserTab* tab);
  void updateEmptyState();

  QStackedWidget* m_stackedWidget;
  QWidget* m_emptyStateWidget;
  QTabWidget* m_tabWidget;
  int m_emptyStateIndex;
  int m_tabsIndex;
  QList<BrowserTab*> m_tabs;
  BrowserTab* m_currentTab;
};
