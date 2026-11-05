#include "BrowserWorkspace.h"

#include "lifecycle/BrowserTab.h"

#include <QLabel>
#include <QPushButton>
#include <QStyle>
#include <QVBoxLayout>

BrowserWorkspace::BrowserWorkspace(QWidget* parent)
  : QStackedWidget(parent)
  , m_emptyStateWidget(nullptr) {
  setObjectName("browserWorkspace");
  setupEmptyState();
}

void BrowserWorkspace::setupEmptyState() {
  m_emptyStateWidget = new QWidget();
  m_emptyStateWidget->setObjectName("emptyState");

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
      new QLabel("Select a provider from the sidebar to open your first workspace tab.",
                 m_emptyStateWidget);
  subtitleLabel->setAlignment(Qt::AlignCenter);
  QFont subFont = subtitleLabel->font();
  subFont.setPointSize(11);
  subtitleLabel->setFont(subFont);
  layout->addWidget(subtitleLabel);
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
