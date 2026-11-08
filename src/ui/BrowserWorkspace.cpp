#include "BrowserWorkspace.h"

#include "lifecycle/BrowserTab.h"
#include "providers/Provider.h"
#include "providers/ProviderIcons.h"
#include "providers/ProviderRegistry.h"

#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QStyle>
#include <QToolButton>
#include <QVBoxLayout>
#include <QWidget>

BrowserWorkspace::BrowserWorkspace(QWidget* parent)
  : QStackedWidget(parent)
  , m_emptyStateWidget(nullptr) {
  setObjectName("browserWorkspace");
  setupEmptyState();
  addWidget(m_emptyStateWidget);
  setCurrentWidget(m_emptyStateWidget);
}

void BrowserWorkspace::setupEmptyState() {
  m_emptyStateWidget = new QWidget();
  m_emptyStateWidget->setObjectName("emptyState");

  QVBoxLayout* layout = new QVBoxLayout(m_emptyStateWidget);
  layout->setAlignment(Qt::AlignCenter | Qt::AlignTop);
  layout->setSpacing(24);
  layout->setContentsMargins(40, 40, 40, 40);

  // App identity
  QLabel* iconLabel = new QLabel(m_emptyStateWidget);
  iconLabel->setPixmap(style()->standardIcon(QStyle::SP_FileDialogNewFolder).pixmap(48, 48));
  iconLabel->setAlignment(Qt::AlignCenter);
  layout->addWidget(iconLabel);

  QLabel* titleLabel = new QLabel("Your AI workspace", m_emptyStateWidget);
  titleLabel->setAlignment(Qt::AlignCenter);
  QFont titleFont = titleLabel->font();
  titleFont.setPointSize(20);
  titleFont.setBold(true);
  titleLabel->setFont(titleFont);
  titleLabel->setObjectName("emptyStateTitle");
  layout->addWidget(titleLabel);

  QLabel* subtitleLabel =
      new QLabel("Select a provider to start a conversation.", m_emptyStateWidget);
  subtitleLabel->setAlignment(Qt::AlignCenter);
  QFont subFont = subtitleLabel->font();
  subFont.setPointSize(11);
  subtitleLabel->setFont(subFont);
  subtitleLabel->setObjectName("emptyStateSubtitle");
  layout->addWidget(subtitleLabel);

  // Provider shortcut grid
  QFrame* providerFrame = new QFrame(m_emptyStateWidget);
  providerFrame->setObjectName("emptyStateProviderFrame");
  providerFrame->setMinimumSize(420, 360);
  QGridLayout* gridLayout = new QGridLayout(providerFrame);
  gridLayout->setSpacing(12);
  gridLayout->setContentsMargins(20, 20, 20, 20);

  ProviderRegistry* registry = ProviderRegistry::instance();
  QList<Provider*> providers = registry->providers();

  int col = 0;
  int row = 0;
  int maxCols = 3;

  for (Provider* provider : providers) {
    QToolButton* btn = new QToolButton(providerFrame);
    btn->setObjectName("emptyStateProviderButton");
    btn->setFixedSize(120, 110);
    btn->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    btn->setIcon(provider->icon());
    btn->setIconSize(QSize(32, 32));
    btn->setText(provider->displayName());
    btn->setToolTip(QString("Open %1").arg(provider->displayName()));
    btn->setProperty("providerId", provider->id());

    QFont btnFont = btn->font();
    btnFont.setPointSize(9);
    btn->setFont(btnFont);

    connect(btn, &QToolButton::clicked, this, [this, provider]() {
      emit emptyStateProviderRequested(provider->id());
    });

    gridLayout->addWidget(btn, row, col);
    col++;
    if (col >= maxCols) {
      col = 0;
      row++;
    }
  }

  layout->addWidget(providerFrame, 0, Qt::AlignHCenter);
  layout->addStretch();
}

void BrowserWorkspace::addTab(BrowserTab* tab, const QString& tabId) {
  if (m_tabs.contains(tabId)) {
    return;
  }

  m_tabs.insert(tabId, tab);
  addWidget(tab);

  if (m_tabs.size() == 1) {
    activateTab(tabId);
  }

  emit tabAdded(tab, tabId);
  updateEmptyState();
}

void BrowserWorkspace::removeTab(BrowserTab* tab, const QString& tabId) {
  if (!m_tabs.contains(tabId)) {
    return;
  }

  bool wasCurrent = (m_currentTabId == tabId);

  m_tabs.remove(tabId);
  removeWidget(tab);
  tab->deleteLater();

  if (wasCurrent) {
    m_currentTabId.clear();
    if (!m_tabs.isEmpty()) {
      activateTab(m_tabs.constBegin().key());
    } else {
      setCurrentWidget(m_emptyStateWidget);
    }
  }

  updateEmptyState();
  emit tabRemoved(tabId);
}

void BrowserWorkspace::activateTab(const QString& tabId) {
  if (!m_tabs.contains(tabId)) {
    return;
  }

  BrowserTab* tab = m_tabs.value(tabId);
  if (m_currentTabId == tabId) {
    return;
  }

  m_currentTabId = tabId;
  setCurrentWidget(tab);
  emit tabActivated(tab, tabId);
}

BrowserTab* BrowserWorkspace::currentTab() const {
  return m_tabs.value(m_currentTabId, nullptr);
}

QString BrowserWorkspace::currentTabId() const {
  return m_currentTabId;
}

int BrowserWorkspace::tabCount() const {
  return m_tabs.size();
}

bool BrowserWorkspace::hasTabs() const {
  return !m_tabs.isEmpty();
}

BrowserTab* BrowserWorkspace::tabById(const QString& tabId) const {
  return m_tabs.value(tabId, nullptr);
}

void BrowserWorkspace::showEmptyState() {
  setCurrentWidget(m_emptyStateWidget);
}

void BrowserWorkspace::hideEmptyState() {
  // Handled by activateTab
}

void BrowserWorkspace::updateEmptyState() {
  if (m_tabs.isEmpty()) {
    setCurrentWidget(m_emptyStateWidget);
    m_currentTabId.clear();
  }
}
