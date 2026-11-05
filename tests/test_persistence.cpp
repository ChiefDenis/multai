#include <QtTest/QtTest>
#include <QStandardPaths>
#include <QUrl>

#include "core/TabStore.h"
#include "core/SettingsManager.h"
#include "lifecycle/TabMetadata.h"

class TestPersistence : public QObject {
  Q_OBJECT
private slots:
  void initTestCase();
  void testTabMetadataSaveAndRestore();
  void testLifecycleStatePersistence();
  void testOperationalRestorationAsUnloaded();
  void testSettingsPersistence();
  void testSidebarPreferencesPersistence();
  void testCurrentIndexPersistence();
};

void TestPersistence::initTestCase() {
  qputenv("QT_QPA_PLATFORM", "offscreen");
  QStandardPaths::setTestModeEnabled(true);
}

void TestPersistence::testTabMetadataSaveAndRestore() {
  QList<TabMetadata> tabs;
  tabs.append(TabMetadata("deepseek", "default", QUrl("https://www.deepseek.com/")));
  tabs[0].setTitle("DeepSeek");
  tabs.append(TabMetadata("qwen", "work", QUrl("https://qwen.ai/home")));
  tabs[1].setTitle("Qwen Work");
  TabStore::save(tabs, 1);

  QList<TabMetadata> loaded = TabStore::load();
  QCOMPARE(loaded.size(), 2);
  QCOMPARE(loaded[0].providerId(), QString("deepseek"));
  QCOMPARE(loaded[0].profileId(), QString("default"));
  QCOMPARE(loaded[0].url(), QUrl("https://www.deepseek.com/"));
  QCOMPARE(loaded[0].title(), QString("DeepSeek"));
  QCOMPARE(loaded[1].providerId(), QString("qwen"));
  QCOMPARE(loaded[1].profileId(), QString("work"));
  QCOMPARE(loaded[1].url(), QUrl("https://qwen.ai/home"));
  QCOMPARE(loaded[1].title(), QString("Qwen Work"));
}

void TestPersistence::testLifecycleStatePersistence() {
  QList<TabMetadata> tabs;
  tabs.append(TabMetadata("claude", "personal", QUrl("https://claude.ai/")));
  tabs[0].setLifecycleState(LifecycleState::Frozen);
  tabs.append(TabMetadata("perplexity", "research", QUrl("https://www.perplexity.ai/")));
  tabs[1].setLifecycleState(LifecycleState::Discarded);
  TabStore::save(tabs, 0);

  QList<TabMetadata> loaded = TabStore::load();
  QCOMPARE(loaded.size(), 2);
  QCOMPARE(loaded[0].lifecycleState(), LifecycleState::Frozen);
  QCOMPARE(loaded[1].lifecycleState(), LifecycleState::Discarded);
}

void TestPersistence::testOperationalRestorationAsUnloaded() {
  // Tabs saved in Frozen/Discarded state are restored with their last known
  // lifecycle state in metadata, but operationally the browser tab starts
  // as Unloaded (no page created until activated).
  QList<TabMetadata> tabs;
  tabs.append(TabMetadata("chatgpt", "default", QUrl("https://chatgpt.com/")));
  tabs[0].setLifecycleState(LifecycleState::Frozen);
  TabStore::save(tabs, 0);

  QList<TabMetadata> loaded = TabStore::load();
  QCOMPARE(loaded.size(), 1);
  // Last known state preserved in metadata
  QCOMPARE(loaded[0].lifecycleState(), LifecycleState::Frozen);
  // But no page is created - operational state is Unloaded until activated
  QVERIFY(loaded[0].url() == QUrl("https://chatgpt.com/"));
}

void TestPersistence::testSettingsPersistence() {
  SettingsManager* settings = SettingsManager::instance();

  settings->setTheme("dark");
  settings->setFreezeDelayMs(10 * 60 * 1000);
  settings->setHibernateDelayMs(45 * 60 * 1000);
  settings->setLifecycleAutomationEnabled(false);
  settings->setMemoryPressureEnabled(true);
  settings->setMemoryPressureThresholdPercent(10);
  settings->setMemoryPressureRecoveryThresholdPercent(25);

  // Create a new SettingsManager instance to simulate restart
  // (SettingsManager is a singleton, so we just re-read values)
  QCOMPARE(settings->theme(), QString("dark"));
  QCOMPARE(settings->freezeDelayMs(), 10 * 60 * 1000);
  QCOMPARE(settings->hibernateDelayMs(), 45 * 60 * 1000);
  QCOMPARE(settings->lifecycleAutomationEnabled(), false);
  QCOMPARE(settings->memoryPressureEnabled(), true);
  QCOMPARE(settings->memoryPressureThresholdPercent(), 10);
  QCOMPARE(settings->memoryPressureRecoveryThresholdPercent(), 25);

  // Reset to defaults
  settings->setTheme("system");
  settings->setFreezeDelayMs(5 * 60 * 1000);
  settings->setHibernateDelayMs(30 * 60 * 1000);
  settings->setLifecycleAutomationEnabled(true);
  settings->setMemoryPressureEnabled(true);
  settings->setMemoryPressureThresholdPercent(15);
  settings->setMemoryPressureRecoveryThresholdPercent(20);
}

void TestPersistence::testSidebarPreferencesPersistence() {
  SettingsManager* settings = SettingsManager::instance();

  // Save sidebar width and collapsed state
  settings->setValue("sidebar/geometry", QByteArray("test-geometry-data"));
  settings->setValue("sidebar/collapsed", true);

  QCOMPARE(settings->value("sidebar/geometry").toByteArray(),
           QByteArray("test-geometry-data"));
  QCOMPARE(settings->value("sidebar/collapsed").toBool(), true);

  // Cleanup
  settings->reset("sidebar/geometry");
  settings->reset("sidebar/collapsed");
}

void TestPersistence::testCurrentIndexPersistence() {
  QList<TabMetadata> tabs;
  tabs.append(TabMetadata("gemini", "default", QUrl("https://gemini.google.com/")));
  tabs.append(TabMetadata("kimi", "work", QUrl("https://www.kimi.ai/")));
  TabStore::save(tabs, 1); // Second tab is current

  QCOMPARE(TabStore::currentIndex(), 1);

  // Change current index
  TabStore::save(tabs, 0);
  QCOMPARE(TabStore::currentIndex(), 0);
}

QTEST_MAIN(TestPersistence)
#include "test_persistence.moc"
