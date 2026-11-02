#pragma once

#include <QList>
#include <QMap>
#include <QObject>
#include "TabMetadata.h"

class LifecycleTarget;
class MemoryMonitor;

class LifecycleManager : public QObject {
  Q_OBJECT
public:
  static LifecycleManager* instance();

  void applySettings();

  void registerTab(LifecycleTarget* tab);
  void unregisterTab(LifecycleTarget* tab);

  void onTabActivated(LifecycleTarget* tab);
  void onTabDeactivated(LifecycleTarget* tab);

  void freezeTab(LifecycleTarget* tab);
  void hibernateTab(LifecycleTarget* tab);

  void setFreezeDelay(qint64 msecs);
  void setHibernateDelay(qint64 msecs);
  qint64 freezeDelay() const { return m_freezeDelay; }
  qint64 hibernateDelay() const { return m_hibernateDelay; }

  void setAutomationEnabled(bool enabled);
  bool automationEnabled() const { return m_automationEnabled; }

  LifecycleTarget* currentTab() const { return m_currentTab; }
  int registeredTabCount() const { return m_lastActiveTimes.size(); }
  QList<LifecycleTarget*> registeredTabs() const;

  LifecycleTarget* selectLeastRecentlyUsed() const;
  int hibernateLeastRecentlyUsed(int maxTabs = 1);

signals:
  void lifecycleStateChanged(LifecycleTarget* tab, LifecycleState state);
  void memoryPressureDetected(int availablePercent);
  void tabHibernated(LifecycleTarget* tab);

private slots:
  void onMemoryPressure(int availablePercent);

private:
  explicit LifecycleManager(QObject* parent = nullptr);
  ~LifecycleManager() override;

  static qint64 now();

  QMap<LifecycleTarget*, qint64> m_lastActiveTimes;
  LifecycleTarget* m_currentTab;
  MemoryMonitor* m_memoryMonitor;
  qint64 m_freezeDelay;
  qint64 m_hibernateDelay;
  bool m_automationEnabled;
};
