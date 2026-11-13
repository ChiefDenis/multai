#include <QtTest/QtTest>
#include <QStandardPaths>
#include <QVariantList>
#include <QVariantMap>
#include "SettingsManager.h"
#include "TabStore.h"
#include "providers/Provider.h"
#include "providers/ProviderRegistry.h"

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
  void testSchemaVersionSaved();
  void testFutureSchemaIgnored();
  void testMissingProviderSkipped();
  void testUnrelatedUrlFallsBack();
  void testAllowedSubdomainUrlsPreserved();
  void testSessionTabLimit();
  void testRecordsStoreNoCredentials();
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
  QCOMPARE(loaded.size(), 2);
  QCOMPARE(loaded.at(0).providerId(), QString("deepseek"));
  // A malformed URL falls back to the provider's configured chat URL.
  QCOMPARE(loaded.at(0).url(), ProviderRegistry::instance()->providerById("deepseek")->url());
  QCOMPARE(loaded.at(1).providerId(), QString("claude"));
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

void TestTabStore::testSchemaVersionSaved() {
  QList<TabMetadata> tabs;
  tabs.append(TabMetadata("qwen", "default", QUrl("https://chat.qwen.ai/")));
  TabStore::save(tabs, 0);

  QCOMPARE(TabStore::savedVersion(), TabStore::kSessionVersion);
  QCOMPARE(SettingsManager::instance()->value("workspace/sessionVersion").toInt(),
           TabStore::kSessionVersion);
}

void TestTabStore::testFutureSchemaIgnored() {
  SettingsManager::instance()->setValue("workspace/sessionVersion", 99);
  QVariantList stored;
  QVariantMap entry;
  entry["providerId"] = "qwen";
  entry["profileId"] = "default";
  entry["url"] = "https://chat.qwen.ai/";
  stored.append(entry);
  SettingsManager::instance()->setValue("workspace/openTabs", stored);

  QCOMPARE(TabStore::load().size(), 0);
}

void TestTabStore::testMissingProviderSkipped() {
  QVariantList stored;
  QVariantMap entry;
  entry["providerId"] = "does-not-exist";
  entry["profileId"] = "default";
  entry["url"] = "https://example.org/";
  stored.append(entry);
  QVariantMap valid;
  valid["providerId"] = "kimi";
  valid["profileId"] = "default";
  valid["url"] = "https://kimi.ai/chat";
  stored.append(valid);
  SettingsManager::instance()->setValue("workspace/openTabs", stored);

  const QList<TabMetadata> loaded = TabStore::load();
  QCOMPARE(loaded.size(), 1);
  QCOMPARE(loaded.at(0).providerId(), QString("kimi"));
}

void TestTabStore::testUnrelatedUrlFallsBack() {
  QVariantList stored;
  QVariantMap entry;
  entry["providerId"] = "claude";
  entry["profileId"] = "default";
  entry["url"] = "https://attacker.example.net/";
  stored.append(entry);
  SettingsManager::instance()->setValue("workspace/openTabs", stored);

  const QList<TabMetadata> loaded = TabStore::load();
  QCOMPARE(loaded.size(), 1);
  QCOMPARE(loaded.at(0).url(), ProviderRegistry::instance()->providerById("claude")->url());
}

void TestTabStore::testAllowedSubdomainUrlsPreserved() {
  QList<TabMetadata> tabs;
  tabs.append(TabMetadata("chatgpt", "default",
                          QUrl("https://chatgpt.com/c/123e4567-e89b-12d3-a456-426614174000")));
  tabs.append(TabMetadata("gemini", "default",
                          QUrl("https://gemini.google.com/app/abcdef")));
  tabs.append(TabMetadata("kimi", "default", QUrl("https://kimi.com/chat/123")));
  TabStore::save(tabs, 0);

  const QList<TabMetadata> loaded = TabStore::load();
  QCOMPARE(loaded.size(), 3);
  QCOMPARE(loaded.at(0).url(), tabs.at(0).url());
  QCOMPARE(loaded.at(1).url(), tabs.at(1).url());
  QCOMPARE(loaded.at(2).url(), tabs.at(2).url());
}

void TestTabStore::testSessionTabLimit() {
  QList<TabMetadata> tabs;
  for (int i = 0; i < 60; ++i) {
    tabs.append(TabMetadata("deepseek", "default", QUrl("https://chat.deepseek.com/")));
  }
  TabStore::save(tabs, 0);

  QCOMPARE(TabStore::load().size(), 50);
}

void TestTabStore::testRecordsStoreNoCredentials() {
  QList<TabMetadata> tabs;
  tabs.append(TabMetadata("copilot", "work",
                          QUrl("https://copilot.microsoft.com/chats/abc")));
  TabStore::save(tabs, 0);

  const QVariantList stored =
      SettingsManager::instance()->value("workspace/openTabs").toList();
  QCOMPARE(stored.size(), 1);
  const QVariantMap record = stored.at(0).toMap();
  const QStringList allowed = {"providerId", "profileId", "url", "title", "lifecycleState"};
  for (auto it = record.begin(); it != record.end(); ++it) {
    QVERIFY(allowed.contains(it.key()));
  }
}

QTEST_MAIN(TestTabStore)
#include "test_tabstore.moc"
