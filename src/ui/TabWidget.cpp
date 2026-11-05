#include "TabWidget.h"
#include "LifecycleManager.h"
#include "ProfileSelectionDialog.h"
#include "ProviderSelectionDialog.h"
#include "TabStore.h"
#include "lifecycle/BrowserTab.h"
#include "providers/Provider.h"
#include "providers/ProviderIcons.h"
#include "providers/ProviderRegistry.h"
#include <QLabel>
#include <QPushButton>
#include <QSignalBlocker>
#include <QStackedWidget>
#include <QStyle>
#include <QTabWidget>
#include <QUrl>
#include <QVBoxLayout>

TabWidget::TabWidget(QWidget* parent)
  : QWidget(parent)
  , m_stackedWidget(nullptr)
  , m_emptyStateWidget(nullptr)
  , m_tabWidget(nullptr)
  , m_emptyStateIndex(-1)
  , m_tabsIndex(-1)
  , m_currentTab(nullptr) {
  QVBoxLayout* layout = new QVBoxLayout(this);
  layout->setContentsMargins(0, 0, 0, 0);

  m_stackedWidget = new QStackedWidget(this);
  layout->addWidget(m_stackedWidget);

  setupEmptyState();
  setupTabs();

  m_emptyStateIndex = m_stackedWidget->addWidget(m_emptyStateWidget);
  m_tabsIndex = m_stackedWidget->addWidget(m_tabWidget);

  m_stackedWidget->setCurrentIndex(m_emptyStateIndex);
}

void TabWidget::setupEmptyState() {
  m_emptyStateWidget = new QWidget();

  QVBoxLayout* layout = new QVBoxLayout(m_emptyStateWidget);
  layout->setAlignment(Qt::AlignCenter);
  layout->setSpacing(16);

  QLabel* iconLabel = new QLabel(m_emptyStateWidget);
  iconLabel->setPixmap(style()->standardIcon(QStyle::SP_FileDialogNewFolder).pixmap(64, 64));
  iconLabel->setAlignment(Qt::AlignCenter);
  layout->addWidget(iconLabel);

  QLabel* titleLabel = new QLabel("No Tabs Open", m_emptyStateWidget);
  titleLabel->setAlignment(Qt::AlignCenter);
  QFont titleFont = titleLabel->font();
  titleFont.setPointSize(18);
  titleFont.setBold(true);
  titleLabel->setFont(titleFont);
  layout->addWidget(titleLabel);

  QLabel* subtitleLabel =
      new QLabel("Select an AI provider to open your first workspace tab.", m_emptyStateWidget);
  subtitleLabel->setAlignment(Qt::AlignCenter);
  QFont subFont = subtitleLabel->font();
  subFont.setPointSize(11);
  subtitleLabel->setFont(subFont);
  layout->addWidget(subtitleLabel);

  QPushButton* newTabButton = new QPushButton("New Tab", m_emptyStateWidget);
  newTabButton->setMinimumWidth(180);
  newTabButton->setToolTip("Create a new tab (Ctrl+N)");
  newTabButton->setProperty("default", true);
  connect(newTabButton, &QPushButton::clicked, this, &TabWidget::showProviderSelection);
  layout->addWidget(newTabButton, 0, Qt::AlignCenter);
}

void TabWidget::setupTabs() {
  m_tabWidget = new QTabWidget(this);
  m_tabWidget->setTabsClosable(true);
  m_tabWidget->setMovable(true);

  connect(m_tabWidget, &QTabWidget::tabCloseRequested, this, [this](int index) {
    removeBrowserTab(m_tabs.value(index, nullptr));
  });

  connect(m_tabWidget, &QTabWidget::currentChanged, this, [this](int index) {
    BrowserTab* newTab = (index >= 0 && index < m_tabs.size()) ? m_tabs.at(index) : nullptr;
    BrowserTab* previous = m_currentTab;
    m_currentTab = newTab;

    if (previous && previous != newTab) {
      LifecycleManager::instance()->onTabDeactivated(previous);
    }
    if (newTab) {
      LifecycleManager::instance()->onTabActivated(newTab);
    }
    emit currentTabChanged(newTab);
  });
}

void TabWidget::showProviderSelection() {
  ProviderSelectionDialog dialog(this);
  connect(&dialog, &ProviderSelectionDialog::providerSelected, this, &TabWidget::onProviderSelected);
  dialog.exec();
}

BrowserTab* TabWidget::currentTab() const {
  return m_currentTab;
}

void TabWidget::onProviderSelected(const QString& providerId) {
  Provider* provider = ProviderRegistry::instance()->providerById(providerId);
  if (!provider) {
    return;
  }

  ProfileSelectionDialog dialog(providerId, provider->displayName(), this);
  if (dialog.exec() != QDialog::Accepted) {
    return;
  }

  BrowserTab* tab = new BrowserTab(providerId, dialog.selectedAccountId(), provider->url(),
                                   LifecycleState::Unloaded, this);
  addBrowserTab(tab, provider->displayName());

  LifecycleManager::instance()->registerTab(tab);
  updateEmptyState();
  m_tabWidget->setCurrentWidget(tab);
}

BrowserTab* TabWidget::addBrowserTab(BrowserTab* tab, const QString& title) {
  m_tabs.append(tab);

  const QString tabTitle = title.isEmpty() ? tab->providerId() : title;
  m_tabWidget->addTab(
      tab, providerIcon(tab->providerId(), tab->lifecycleState(), tab->hasError()), tabTitle);

  connect(tab, &BrowserTab::titleChanged, this, [this, tab](const QString& newTitle) {
    const int tabIndex = m_tabs.indexOf(tab);
    if (tabIndex >= 0 && !newTitle.isEmpty()) {
      m_tabWidget->setTabText(tabIndex, newTitle);
      updateTab(tab);
    }
  });

  connect(tab, &BrowserTab::lifecycleStateChanged, this, [this, tab]() { updateTab(tab); });
  connect(tab, &BrowserTab::errorChanged, this, [this, tab]() { updateTab(tab); });
  connect(tab, &BrowserTab::closeRequested, this, [this, tab]() {
    removeBrowserTab(tab);
  });

  updateTab(tab);
  return tab;
}

void TabWidget::updateTab(BrowserTab* tab) {
  const int tabIndex = m_tabs.indexOf(tab);
  if (tabIndex < 0) {
    return;
  }

  m_tabWidget->setTabIcon(
      tabIndex, providerIcon(tab->providerId(), tab->lifecycleState(), tab->hasError()));

  QString status = lifecycleStateToString(tab->lifecycleState());
  if (tab->hasError()) {
    status += " (Error)";
  }
  m_tabWidget->setTabToolTip(
      tabIndex,
      QString("%1 — %2\nStatus: %3").arg(m_tabWidget->tabText(tabIndex), tab->profileId(), status));

  if (m_currentTab == tab) {
    emit currentTabChanged(tab);
  }
}

void TabWidget::removeBrowserTab(BrowserTab* tab) {
  const int index = m_tabs.indexOf(tab);
  if (index < 0) {
    return;
  }

  if (m_currentTab == tab) {
    m_currentTab = nullptr;
    emit currentTabChanged(nullptr);
  }

  LifecycleManager::instance()->unregisterTab(tab);
  tab->closeTab();

  m_tabs.removeAt(index);
  m_tabWidget->removeTab(index);
  tab->deleteLater();

  updateEmptyState();
}

void TabWidget::updateEmptyState() {
  m_stackedWidget->setCurrentIndex(m_tabs.isEmpty() ? m_emptyStateIndex : m_tabsIndex);
}

void TabWidget::restoreTabs() {
  const QList<TabMetadata> stored = TabStore::load();
  if (stored.isEmpty()) {
    updateEmptyState();
    return;
  }

  ProviderRegistry* registry = ProviderRegistry::instance();
  {
    const QSignalBlocker blocker(m_tabWidget);
    for (const TabMetadata& metadata : stored) {
      Provider* provider = registry->providerById(metadata.providerId());
      if (!provider) {
        continue;
      }

      BrowserTab* tab =
          new BrowserTab(metadata.providerId(), metadata.profileId(), metadata.url(),
                         metadata.lifecycleState(), this);
      addBrowserTab(tab, metadata.title().isEmpty() ? provider->displayName() : metadata.title());
      LifecycleManager::instance()->registerTab(tab);
    }
  }

  updateEmptyState();

  if (m_tabs.isEmpty()) {
    return;
  }

  int index = TabStore::currentIndex();
  if (index < 0 || index >= m_tabs.size()) {
    index = 0;
  }
  m_tabWidget->setCurrentIndex(index);
}

void TabWidget::saveTabs() const {
  QList<TabMetadata> tabs;
  for (int i = 0; i < m_tabs.size(); ++i) {
    TabMetadata metadata = m_tabs.at(i)->metadata();
    metadata.setTitle(m_tabWidget->tabText(i));
    tabs.append(metadata);
  }

  TabStore::save(tabs, m_tabWidget->currentIndex());
}
