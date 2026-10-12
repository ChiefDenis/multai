#include "TabWidget.h"
#include "ProviderSelectionDialog.h"
#include <QStackedWidget>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QStyle>

TabWidget::TabWidget(QWidget* parent)
  : QWidget(parent)
  , m_stackedWidget(nullptr)
  , m_emptyStateWidget(nullptr)
  , m_tabsWidget(nullptr)
  , m_emptyStateIndex(-1)
  , m_tabsIndex(-1) {
  QVBoxLayout* layout = new QVBoxLayout(this);
  layout->setContentsMargins(0, 0, 0, 0);

  m_stackedWidget = new QStackedWidget(this);
  layout->addWidget(m_stackedWidget);

  setupEmptyState();
  setupTabs();

  m_emptyStateIndex = m_stackedWidget->addWidget(m_emptyStateWidget);
  m_tabsIndex = m_stackedWidget->addWidget(m_tabsWidget);

  m_stackedWidget->setCurrentIndex(m_emptyStateIndex);
}

void TabWidget::setupEmptyState() {
  m_emptyStateWidget = new QWidget();

  QVBoxLayout* layout = new QVBoxLayout(m_emptyStateWidget);
  layout->setAlignment(Qt::AlignCenter);

  QLabel* iconLabel = new QLabel(m_emptyStateWidget);
  iconLabel->setPixmap(style()->standardIcon(QStyle::SP_FileIcon).pixmap(64, 64));
  iconLabel->setAlignment(Qt::AlignCenter);
  layout->addWidget(iconLabel);

  QLabel* titleLabel = new QLabel("No tabs open", m_emptyStateWidget);
  titleLabel->setAlignment(Qt::AlignCenter);
  QFont titleFont = titleLabel->font();
  titleFont.setPointSize(16);
  titleFont.setBold(true);
  titleLabel->setFont(titleFont);
  layout->addWidget(titleLabel);

  QLabel* subtitleLabel = new QLabel("Create a new tab to get started", m_emptyStateWidget);
  subtitleLabel->setAlignment(Qt::AlignCenter);
  layout->addWidget(subtitleLabel);

  QPushButton* newTabButton = new QPushButton("New Tab", m_emptyStateWidget);
  newTabButton->setMinimumWidth(200);
  newTabButton->setToolTip("Create a new tab");
  connect(newTabButton, &QPushButton::clicked, this, &TabWidget::showProviderSelection);
  layout->addWidget(newTabButton);

  layout->addStretch();
}

void TabWidget::setupTabs() {
  m_tabsWidget = new QWidget();
  QVBoxLayout* layout = new QVBoxLayout(m_tabsWidget);
  layout->setContentsMargins(0, 0, 0, 0);

  QLabel* placeholderLabel = new QLabel("Tabs will appear here", m_tabsWidget);
  placeholderLabel->setAlignment(Qt::AlignCenter);
  layout->addWidget(placeholderLabel);
}

void TabWidget::showProviderSelection() {
  ProviderSelectionDialog dialog(this);
  dialog.exec();
}
