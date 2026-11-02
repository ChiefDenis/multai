#include <QtTest/QtTest>
#include <QStandardPaths>
#include "LifecycleManager.h"
#include "LifecycleTarget.h"
#include "TestClock.h"

class FakeTab : public LifecycleTarget {
public:
  explicit FakeTab(const QString& name)
    : m_name(name)
    , m_state(LifecycleState::Active)
    , m_hibernatable(true) {
  }

  LifecycleState lifecycleState() const override { return m_state; }
  bool isSuspended() const override { return m_state == LifecycleState::Discarded; }
  bool isSuspendable() const override {
    return m_state == LifecycleState::Active || m_state == LifecycleState::Frozen;
  }

  void activate() override { m_state = LifecycleState::Active; }
  void deactivate() override { m_deactivated = true; }
  void freeze() override { m_frozen = true; }

  void hibernate() override {
    if (m_hibernatable) {
      m_state = LifecycleState::Discarded;
    }
  }

  void setHibernatable(bool value) { m_hibernatable = value; }
  void setState(LifecycleState state) { m_state = state; }
  bool wasDeactivated() const { return m_deactivated; }
  bool wasFrozen() const { return m_frozen; }
  QString name() const { return m_name; }

private:
  QString m_name;
  LifecycleState m_state;
  bool m_hibernatable;
  bool m_deactivated = false;
  bool m_frozen = false;
};

class TestLifecycleManager : public QObject {
  Q_OBJECT
private slots:
  void initTestCase();
  void cleanup();
  void testDelaysAreConfigurable();
  void testActivationTracksCurrentTab();
  void testDeactivationPropagates();
  void testLeastRecentlyUsedSelection();
  void testSkipsCurrentTab();
  void testSkipsNonSuspendableTabs();
  void testHibernateLeastRecentlyUsed();
  void testHibernateStopsWhenNotSuspendable();
  void testUnregisterClearsCurrentTab();
  void testMemoryPressureHibernatesLru();
};

void TestLifecycleManager::initTestCase() {
  QStandardPaths::setTestModeEnabled(true);
  TestClock::setTestMode(true);
}

void TestLifecycleManager::cleanup() {
  LifecycleManager* manager = LifecycleManager::instance();
  const QList<LifecycleTarget*> tabs = manager->registeredTabs();
  for (LifecycleTarget* tab : tabs) {
    manager->unregisterTab(tab);
  }
  TestClock::instance()->reset();
}

void TestLifecycleManager::testDelaysAreConfigurable() {
  LifecycleManager* manager = LifecycleManager::instance();
  manager->setFreezeDelay(1234);
  manager->setHibernateDelay(5678);
  QCOMPARE(manager->freezeDelay(), qint64(1234));
  QCOMPARE(manager->hibernateDelay(), qint64(5678));
}

void TestLifecycleManager::testActivationTracksCurrentTab() {
  LifecycleManager* manager = LifecycleManager::instance();
  FakeTab tab("a");

  manager->registerTab(&tab);
  QCOMPARE(manager->registeredTabCount(), 1);
  QVERIFY(manager->currentTab() == nullptr);

  manager->onTabActivated(&tab);
  QVERIFY(manager->currentTab() == &tab);
  QCOMPARE(tab.lifecycleState(), LifecycleState::Active);
}

void TestLifecycleManager::testDeactivationPropagates() {
  LifecycleManager* manager = LifecycleManager::instance();
  FakeTab tab("a");

  manager->registerTab(&tab);
  manager->onTabActivated(&tab);
  manager->onTabDeactivated(&tab);

  QVERIFY(tab.wasDeactivated());
  QVERIFY(manager->currentTab() == nullptr);
}

void TestLifecycleManager::testLeastRecentlyUsedSelection() {
  LifecycleManager* manager = LifecycleManager::instance();
  FakeTab first("first");
  FakeTab second("second");
  FakeTab third("third");

  manager->registerTab(&first);
  TestClock::instance()->advance(1000);
  manager->registerTab(&second);
  TestClock::instance()->advance(1000);
  manager->registerTab(&third);

  QVERIFY(manager->selectLeastRecentlyUsed() == &first);

  manager->onTabActivated(&third);
  QVERIFY(manager->selectLeastRecentlyUsed() == &first);

  manager->onTabActivated(&second);
  QVERIFY(manager->selectLeastRecentlyUsed() == &first);
}

void TestLifecycleManager::testSkipsCurrentTab() {
  LifecycleManager* manager = LifecycleManager::instance();
  FakeTab only("only");

  manager->registerTab(&only);
  manager->onTabActivated(&only);

  QVERIFY(manager->selectLeastRecentlyUsed() == nullptr);
}

void TestLifecycleManager::testSkipsNonSuspendableTabs() {
  LifecycleManager* manager = LifecycleManager::instance();
  FakeTab loading("loading");
  FakeTab active("active");

  loading.setState(LifecycleState::Loading);
  manager->registerTab(&loading);
  TestClock::instance()->advance(1000);
  manager->registerTab(&active);

  QVERIFY(manager->selectLeastRecentlyUsed() == &active);
}

void TestLifecycleManager::testHibernateLeastRecentlyUsed() {
  LifecycleManager* manager = LifecycleManager::instance();
  FakeTab first("first");
  FakeTab second("second");

  manager->registerTab(&first);
  TestClock::instance()->advance(1000);
  manager->registerTab(&second);
  manager->onTabActivated(&second);

  QCOMPARE(manager->hibernateLeastRecentlyUsed(1), 1);
  QVERIFY(first.isSuspended());
  QVERIFY(!second.isSuspended());
}

void TestLifecycleManager::testHibernateStopsWhenNotSuspendable() {
  LifecycleManager* manager = LifecycleManager::instance();
  FakeTab stuck("stuck");
  stuck.setHibernatable(false);

  manager->registerTab(&stuck);

  QCOMPARE(manager->hibernateLeastRecentlyUsed(3), 0);
  QVERIFY(!stuck.isSuspended());
}

void TestLifecycleManager::testUnregisterClearsCurrentTab() {
  LifecycleManager* manager = LifecycleManager::instance();
  FakeTab tab("a");

  manager->registerTab(&tab);
  manager->onTabActivated(&tab);
  manager->unregisterTab(&tab);

  QVERIFY(manager->currentTab() == nullptr);
  QCOMPARE(manager->registeredTabCount(), 0);
}

void TestLifecycleManager::testMemoryPressureHibernatesLru() {
  LifecycleManager* manager = LifecycleManager::instance();
  manager->setAutomationEnabled(true);

  FakeTab first("first");
  FakeTab second("second");

  manager->registerTab(&first);
  TestClock::instance()->advance(1000);
  manager->registerTab(&second);
  manager->onTabActivated(&second);

  QSignalSpy spy(manager, &LifecycleManager::memoryPressureDetected);
  QMetaObject::invokeMethod(manager, "onMemoryPressure", Qt::DirectConnection, Q_ARG(int, 5));

  QCOMPARE(spy.count(), 1);
  QCOMPARE(spy.at(0).at(0).toInt(), 5);
  QVERIFY(first.isSuspended());
  QVERIFY(!second.isSuspended());
}

QTEST_MAIN(TestLifecycleManager)
#include "test_lifecyclemanager.moc"
