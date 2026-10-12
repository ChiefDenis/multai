#pragma once

#include <QWidget>
#include <QStackedWidget>

class QWidget;
class QVBoxLayout;
class QLabel;
class QPushButton;
class ProviderSelectionDialog;

class TabWidget : public QWidget {
  Q_OBJECT
public:
  explicit TabWidget(QWidget* parent = nullptr);
  ~TabWidget() override = default;

  void showProviderSelection();

private:
  void setupEmptyState();
  void setupTabs();

  QStackedWidget* m_stackedWidget;
  QWidget* m_emptyStateWidget;
  QWidget* m_tabsWidget;
  int m_emptyStateIndex;
  int m_tabsIndex;
};
