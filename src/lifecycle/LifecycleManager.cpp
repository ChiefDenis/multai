#include "LifecycleManager.h"
#include "LifecycleTarget.h"
#include "MemoryMonitor.h"
#include "SettingsManager.h"
#include "TestClock.h"

#include <QDateTime>

LifecycleManager::LifecycleManager(QObject* parent)
  : QObject(parent)
  , m_currentTab(nullptr)
  , m_memoryMonitor(new MemoryMonitor(this))
  , m_freezeDelay(5 * 60 * 1000)
  , m_hibernateDelay(30 * 60 * 1000)
  , m_automationEnabled(true) {
  connect(m_memoryMonitor, &MemoryMonitor::memoryPressureDetected, this,
          &LifecycleManager::onMemoryPressure);
  applySettings();
}

LifecycleManager::~LifecycleManager() = default;

LifecycleManager* LifecycleManager::instance() {
  static LifecycleManager instance;
  return &instance;
}

qint64 LifecycleManager::now() {
  if (TestClock::isTestMode()) {
    return TestClock::instance()->elapsed();
  }
  return QDateTime::currentMSecsSinceEpoch();
}

void LifecycleManager::applySettings() {
  SettingsManager* settings = SettingsManager::instance();

  m_freezeDelay = settings->freezeDelayMs();
  m_hibernateDelay = settings->hibernateDelayMs();
  m_automationEnabled = settings->lifecycleAutomationEnabled();

  if (m_automationEnabled && settings->memoryPressureEnabled()) {
    m_memoryMonitor->start();
  } else {
    m_memoryMonitor->stop();
  }
}

void LifecycleManager::registerTab(LifecycleTarget* tab) {
  if (tab) {
    m_lastActiveTimes.insert(tab, now());
  }
}

void LifecycleManager::unregisterTab(LifecycleTarget* tab) {
  m_lastActiveTimes.remove(tab);
  if (m_currentTab == tab) {
    m_currentTab = nullptr;
  }
}

void LifecycleManager::onTabActivated(LifecycleTarget* tab) {
  if (!tab) {
    return;
  }

  m_currentTab = tab;
  m_lastActiveTimes.insert(tab, now());
  tab->activate();
}

void LifecycleManager::onTabDeactivated(LifecycleTarget* tab) {
  if (!tab) {
    return;
  }

  if (m_currentTab == tab) {
    m_currentTab = nullptr;
  }
  m_lastActiveTimes.insert(tab, now());
  tab->deactivate();
}

void LifecycleManager::freezeTab(LifecycleTarget* tab) {
  if (tab && tab != m_currentTab) {
    tab->freeze();
  }
}

void LifecycleManager::hibernateTab(LifecycleTarget* tab) {
  if (tab && tab != m_currentTab) {
    tab->hibernate();
  }
}

void LifecycleManager::setFreezeDelay(qint64 msecs) {
  m_freezeDelay = msecs;
}

void LifecycleManager::setHibernateDelay(qint64 msecs) {
  m_hibernateDelay = msecs;
}

void LifecycleManager::setAutomationEnabled(bool enabled) {
  m_automationEnabled = enabled;

  if (enabled) {
    m_memoryMonitor->start();
  } else {
    m_memoryMonitor->stop();
  }
}

QList<LifecycleTarget*> LifecycleManager::registeredTabs() const {
  return m_lastActiveTimes.keys();
}

LifecycleTarget* LifecycleManager::selectLeastRecentlyUsed() const {
  LifecycleTarget* candidate = nullptr;
  qint64 oldest = 0;

  for (auto it = m_lastActiveTimes.constBegin(); it != m_lastActiveTimes.constEnd(); ++it) {
    LifecycleTarget* tab = it.key();
    if (tab == m_currentTab || !tab->isSuspendable()) {
      continue;
    }
    if (!candidate || it.value() < oldest) {
      candidate = tab;
      oldest = it.value();
    }
  }

  return candidate;
}

int LifecycleManager::hibernateLeastRecentlyUsed(int maxTabs) {
  int hibernated = 0;

  while (hibernated < maxTabs) {
    LifecycleTarget* tab = selectLeastRecentlyUsed();
    if (!tab) {
      break;
    }

    m_lastActiveTimes.insert(tab, now());
    tab->hibernate();

    if (!tab->isSuspended()) {
      break;
    }

    emit tabHibernated(tab);
    ++hibernated;
  }

  return hibernated;
}

void LifecycleManager::onMemoryPressure(int availablePercent) {
  if (!m_automationEnabled) {
    return;
  }

  emit memoryPressureDetected(availablePercent);
  hibernateLeastRecentlyUsed(1);
}
