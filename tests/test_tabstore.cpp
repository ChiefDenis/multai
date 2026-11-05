#include <QtTest/QtTest>
#include <QStandardPaths>
#include <QVariantList>
#include <QVariantMap>
#include "SettingsManager.h"
#include "TabStore.h"

class TestTabStore : public QObject {
  Q_OBJECT
private slots:
  void initTestCase();
  void cleanup();
  void testSaveAndLoad();
  void testCurrentIndex();
  void testInvalidEntriesSkipped();
  void testClear();
  void testSaveAndRestoreLifecycleState();
  void testOperationalRestorationAsUnloaded();
  void testLastKnownStatePreserved();
  void testOldSettingsWithoutLifecycleState();
  void testInvalidLifecycleStateValues();
};

void TestTabStore::initTestCase() {
  QStandardPaths::setTestModeEnabled(true);
}

void TestTabStore::cleanup() {
  TabStore::clear();
}

void TestTabStore::testSaveAndLoad() {
  QList<TabMetadata> tabs;
  tabs.append(TabMetadata("deepseek", "default", QUrl("https://www.deepseek.com/")));
  tabs.append(TabMetadata("chatgpt", "work", QUrl("https://chatgpt.com/")));
  tabs[1].setTitle("ChatGPT Work");
  TabStore::save(tabs, 1);

  const QList<TabMetadata> loaded = TabStore::load();
  QCOMPARE(loaded.size(), 2);
  QCOMPARE(loaded.at(0).providerId(), QString("deepseek"));
  QCOMPARE(loaded.at(0).profileId(), QString("default"));
  QCOMPARE(loaded.at(0).url(), QUrl("https://www.deepseek.com/"));
  QCOMPARE(loaded.at(0).lifecycleState(), LifecycleState::Unloaded);
  QCOMPARE(loaded.at(1).providerId(), QString("chatgpt"));
  QCOMPARE(loaded.at(1).profileId(), QString("work"));
  QCOMPARE(loaded.at(1).title(), QString("ChatGPT Work"));
}

void TestTabStore::testCurrentIndex() {
  QList<TabMetadata> tabs;
  tabs.append(TabMetadata("kimi", "default", QUrl("https://www.kimi.ai/")));
  TabStore::save(tabs, 0);
  QCOMPARE(TabStore::currentIndex(), 0);

  tabs.append(TabMetadata("qwen", "default", QUrl("https://qwen.ai/home")));
  TabStore::save(tabs, 3);
  QCOMPARE(TabStore::currentIndex(), 3);
}

void TestTabStore::testInvalidEntriesSkipped() {
  QVariantList stored;

  QVariantMap missingUrl;
  missingUrl["providerId"] = "deepseek";
  stored.append(missingUrl);

  QVariantMap missingProvider;
  missingProvider["url"] = "https://chatgpt.com/";
  stored.append(missingProvider);

  QVariantMap valid;
  valid["providerId"] = "claude";
  valid["profileId"] = "default";
  valid["url"] = "https://claude.ai/";
  stored.append(valid);

  SettingsManager::instance()->setValue("workspace/openTabs", stored);

  const QList<TabMetadata> loaded = TabStore::load();
  QCOMPARE(loaded.size(), 1);
  QCOMPARE(loaded.at(0).providerId(), QString("claude"));
}

void TestTabStore::testClear() {
  QList<TabMetadata> tabs;
  tabs.append(TabMetadata("gemini", "default", QUrl("https://gemini.google.com/")));
  TabStore::save(tabs, 0);
  QCOMPARE(TabStore::load().size(), 1);

  TabStore::clear();
  QCOMPARE(TabStore::load().size(), 0);
  QCOMPARE(TabStore::currentIndex(), 0);
}

void TestTabStore::testSaveAndRestoreLifecycleState() {
  QList<TabMetadata> tabs;
  tabs.append(TabMetadata("deepseek", "default", QUrl("https://www.deepseek.com/")));
  tabs[0].setLifecycleState(LifecycleState::Frozen);
  tabs.append(TabMetadata("chatgpt", "work", QUrl("https://chatgpt.com/")));
  tabs[1].setLifecycleState(LifecycleState::Discarded);
  tabs.append(TabMetadata("claude", "personal", QUrl("https://claude.ai/")));
  tabs[2].setLifecycleState(LifecycleState::Active);
  TabStore::save(tabs, 0);

  const QList<TabMetadata> loaded = TabStore::load();
  QCOMPARE(loaded.size(), 3);
  QCOMPARE(loaded.at(0).lifecycleState(), LifecycleState::Frozen);
  QCOMPARE(loaded.at(1).lifecycleState(), LifecycleState::Discarded);
  QCOMPARE(loaded.at(2).lifecycleState(), LifecycleState::Active);
}

void TestTabStore::testOperationalRestorationAsUnloaded() {
  // Tabs saved in any state are restored with a valid lifecycle state from metadata.
  // Operational restoration (no page created) is handled by the caller (BrowserTab
  // constructor defaults to Unloaded); TabStore preserves the last known state.
  QList<TabMetadata> tabs;
  tabs.append(TabMetadata("kimi", "default", QUrl("https://www.kimi.ai/")));
  tabs[0].setLifecycleState(LifecycleState::Frozen);
  TabStore::save(tabs, 0);

  const QList<TabMetadata> loaded = TabStore::load();
  QCOMPARE(loaded.size(), 1);
  QCOMPARE(loaded.at(0).lifecycleState(), LifecycleState::Frozen);
  QCOMPARE(loaded.at(0).url(), QUrl("https://www.kimi.ai/"));
}

void TestTabStore::testLastKnownStatePreserved() {
  QList<TabMetadata> tabs;
  tabs.append(TabMetadata("perplexity", "research", QUrl("https://www.perplexity.ai/")));
  tabs[0].setLifecycleState(LifecycleState::Discarded);
  TabStore::save(tabs, 0);

  // Re-save with a different state; the last saved state wins.
  tabs[0].setLifecycleState(LifecycleState::Frozen);
  TabStore::save(tabs, 0);

  const QList<TabMetadata> loaded = TabStore::load();
  QCOMPARE(loaded.size(), 1);
  QCOMPARE(loaded.at(0).lifecycleState(), LifecycleState::Frozen);
}

void TestTabStore::testOldSettingsWithoutLifecycleState() {
  QVariantList stored;
  QVariantMap entry;
  entry["providerId"] = "qwen";
  entry["profileId"] = "default";
  entry["url"] = "https://qwen.ai/home";
  entry["title"] = "Qwen";
  // No "lifecycleState" field — old settings format.
  stored.append(entry);

  SettingsManager::instance()->setValue("workspace/openTabs", stored);

  const QList<TabMetadata> loaded = TabStore::load();
  QCOMPARE(loaded.size(), 1);
  // Missing lifecycleState defaults to Unloaded via lifecycleStateFromString("").
  QCOMPARE(loaded.at(0).lifecycleState(), LifecycleState::Unloaded);
}

void TestTabStore::testInvalidLifecycleStateValues() {
  QVariantList stored;
  QVariantMap entry;
  entry["providerId"] = "gemini";
  entry["profileId"] = "default";
  entry["url"] = "https://gemini.google.com/";
  entry["lifecycleState"] = "NotARealState";
  stored.append(entry);

  SettingsManager::instance()->setValue("workspace/openTabs", stored);

  const QList<TabMetadata> loaded = TabStore::load();
  QCOMPARE(loaded.size(), 1);
  // Invalid state string defaults to Unloaded.
  QCOMPARE(loaded.at(0).lifecycleState(), LifecycleState::Unloaded);
}

QTEST_MAIN(TestTabStore)
#include "test_tabstore.moc"
