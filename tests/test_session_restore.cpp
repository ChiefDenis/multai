#include <QtTest/QtTest>

#include <QApplication>
#include <QDir>
#include <QLabel>
#include <QStandardPaths>
#include <QToolButton>
#include <QVariantList>
#include <QVariantMap>

#include "BrowserTab.h"
#include "BrowserWorkspace.h"
#include "MainWindow.h"
#include "Profile.h"
#include "ProfileManager.h"
#include "ProfileStorage.h"
#include "Provider.h"
#include "ProviderRegistry.h"
#include "SettingsManager.h"
#include "Sidebar.h"
#include "TabStore.h"
#include "Theme.h"

class TestSessionRestore : public QObject {
  Q_OBJECT
private slots:
  void initTestCase();
  void init();
  void cleanup();

  void testRestoreOrderProfilesUrlsAndActiveTab();
  void testInactiveRestoredTabsStayLazy();
  void testSaveOnOpenSelectReorderAndClose();
  void testEmptySessionStartsClean();
  void testMalformedSessionIsHandled();
  void testFutureSchemaIsNotOverwritten();
  void testDeletedProfileFallsBack();
  void testMissingProviderIsSkipped();
  void testSessionRecordsContainNoCredentials();
  void testShutdownDuringLoadKeepsSession();

private:
  void clearProfiles();
  QList<TabMetadata> storedTabs();
};

void TestSessionRestore::initTestCase() {
  qputenv("QT_QPA_PLATFORM", "offscreen");
  QStandardPaths::setTestModeEnabled(true);
  clearProfiles();
}

void TestSessionRestore::init() {
  TabStore::clear();
  SettingsManager::instance()->setTheme("light");
  Theme::applyTheme("light");
}

void TestSessionRestore::cleanup() {
  TabStore::clear();
  clearProfiles();
}

void TestSessionRestore::clearProfiles() {
  ProfileManager* manager = ProfileManager::instance();
  const QList<Profile*> profiles = manager->profiles();
  for (Profile* profile : profiles) {
    while (profile->refCount() > 0) {
      profile->deref();
    }
    manager->deleteProfile(profile);
  }
}

QList<TabMetadata> TestSessionRestore::storedTabs() {
  return TabStore::load();
}

void TestSessionRestore::testRestoreOrderProfilesUrlsAndActiveTab() {
  ProfileManager* profiles = ProfileManager::instance();
  Profile* deepseekWork = profiles->getOrCreateProfile("deepseek", "work");
  Profile* qwenWork = profiles->getOrCreateProfile("qwen", "work");
  Profile* claudePersonal = profiles->getOrCreateProfile("claude", "personal");
  QVERIFY(deepseekWork != nullptr);
  QVERIFY(qwenWork != nullptr);
  QVERIFY(claudePersonal != nullptr);

  const QString deepseekChatUrl = "https://chat.deepseek.com/a/chat/s/abc123";
  const QString qwenChatUrl = "https://chat.qwen.ai/c/xyz789";
  const QUrl claudeFallback = ProviderRegistry::instance()->providerById("claude")->url();

  QList<TabMetadata> tabs;
  tabs.append(TabMetadata("deepseek", "work", QUrl(deepseekChatUrl)));
  tabs.append(TabMetadata("qwen", "work", QUrl(qwenChatUrl)));
  tabs.append(TabMetadata("claude", "personal", QUrl("https://unrelated.example.org/")));
  TabStore::save(tabs, 1);

  {
    MainWindow window;
    window.resize(1100, 720);
    window.show();
    QApplication::processEvents();

    BrowserWorkspace* workspace = window.workspaceForTest();
    QVERIFY(workspace != nullptr);
    const QStringList tabIds = workspace->tabIds();
    QCOMPARE(tabIds.size(), 3);

    QCOMPARE(workspace->tabById(tabIds[0])->providerId(), QString("deepseek"));
    QCOMPARE(workspace->tabById(tabIds[0])->profileId(), QString("work"));
    QCOMPARE(workspace->tabById(tabIds[0])->url(), QUrl(deepseekChatUrl));

    QCOMPARE(workspace->tabById(tabIds[1])->providerId(), QString("qwen"));
    QCOMPARE(workspace->tabById(tabIds[1])->profileId(), QString("work"));
    QCOMPARE(workspace->tabById(tabIds[1])->url(), QUrl(qwenChatUrl));

    QCOMPARE(workspace->tabById(tabIds[2])->providerId(), QString("claude"));
    QCOMPARE(workspace->tabById(tabIds[2])->profileId(), QString("personal"));
    QCOMPARE(workspace->tabById(tabIds[2])->url(), claudeFallback);

    QCOMPARE(workspace->currentTabId(), tabIds[1]);
    QCOMPARE(window.sidebarForTest()->selectedProvider(), QString("qwen"));

    QVERIFY(profiles->profileById("deepseek:work") != nullptr);
    QCOMPARE(profiles->profileById("deepseek:work")->path(), deepseekWork->path());
    QCOMPARE(profiles->profileById("qwen:work")->path(), qwenWork->path());

    QCOMPARE(window.workspaceForTest()->currentTab()->providerId(), QString("qwen"));
    window.close();
    QApplication::processEvents();
  }
}

void TestSessionRestore::testInactiveRestoredTabsStayLazy() {
  QList<TabMetadata> tabs;
  tabs.append(TabMetadata("chatgpt", "default", QUrl("https://chatgpt.com/")));
  tabs.append(TabMetadata("gemini", "default", QUrl("https://gemini.google.com/app")));
  tabs.append(TabMetadata("perplexity", "default", QUrl("https://www.perplexity.ai/")));
  TabStore::save(tabs, 2);

  {
    MainWindow window;
    window.show();
    QApplication::processEvents();

    BrowserWorkspace* workspace = window.workspaceForTest();
    const QStringList tabIds = workspace->tabIds();
    QCOMPARE(tabIds.size(), 3);
    QCOMPARE(workspace->currentTabId(), tabIds[2]);

    BrowserTab* active = workspace->tabById(tabIds[2]);
    QVERIFY(active != nullptr);
    QVERIFY(active->page() != nullptr);
    QVERIFY(active->lifecycleState() != LifecycleState::Unloaded);

    for (int i = 0; i < 2; ++i) {
      BrowserTab* inactive = workspace->tabById(tabIds[i]);
      QVERIFY(inactive != nullptr);
      QCOMPARE(inactive->lifecycleState(), LifecycleState::Unloaded);
      QVERIFY2(inactive->page() == nullptr,
               "Restored inactive tabs must not create a WebEngine page");
      QVERIFY(!inactive->pageCreated());
    }

    window.close();
    QApplication::processEvents();
  }
}

void TestSessionRestore::testSaveOnOpenSelectReorderAndClose() {
  {
    MainWindow window;
    window.show();
    QApplication::processEvents();

    window.openProviderForTest("deepseek");
    QApplication::processEvents();
    QVERIFY(TabStore::load().size() == 1);
    QCOMPARE(TabStore::load().at(0).providerId(), QString("deepseek"));

    window.openProviderForTest("qwen");
    QApplication::processEvents();
    QCOMPARE(TabStore::load().size(), 2);
    QCOMPARE(TabStore::currentIndex(), 1);

    const QStringList ids = window.workspaceForTest()->tabIds();
    QCOMPARE(ids.size(), 2);

    window.selectTabForTest(ids[0]);
    QApplication::processEvents();
    QCOMPARE(TabStore::currentIndex(), 0);

    QVERIFY(window.workspaceForTest()->moveTab(ids[0], 1));
    QApplication::processEvents();
    QList<TabMetadata> reordered = TabStore::load();
    QCOMPARE(reordered.size(), 2);
    QCOMPARE(reordered.at(0).providerId(), QString("qwen"));
    QCOMPARE(reordered.at(1).providerId(), QString("deepseek"));
    QCOMPARE(TabStore::currentIndex(), 1);

    const QString closedId = window.workspaceForTest()->tabIds().at(1);
    const bool invoked =
        QMetaObject::invokeMethod(&window, "onTabClosed", Qt::DirectConnection,
                                  Q_ARG(QString, closedId));
    QVERIFY(invoked);
    QApplication::processEvents();

    QList<TabMetadata> afterClose = TabStore::load();
    QCOMPARE(afterClose.size(), 1);
    QCOMPARE(afterClose.at(0).providerId(), QString("qwen"));

    window.close();
    QApplication::processEvents();
  }
}

void TestSessionRestore::testEmptySessionStartsClean() {
  TabStore::clear();
  SettingsManager::instance()->reset("workspace/openTabs");
  SettingsManager::instance()->reset("workspace/currentTabIndex");
  SettingsManager::instance()->reset("workspace/sessionVersion");

  MainWindow window;
  window.show();
  QApplication::processEvents();

  QCOMPARE(window.workspaceForTest()->tabCount(), 0);
  QCOMPARE(window.workspaceForTest()->currentTabId(), QString());
  QVERIFY(window.sidebarForTest()->selectedProvider().isEmpty());
  QCOMPARE(window.statusTabInfoLabelForTest()->text(), QString("No tabs open"));
}

void TestSessionRestore::testMalformedSessionIsHandled() {
  QVariantList stored;
  stored.append(QVariant("not-a-map"));
  QVariantMap missingProvider;
  missingProvider["providerId"] = "not-a-provider";
  missingProvider["url"] = "https://example.org/";
  stored.append(missingProvider);
  QVariantMap validProviderWithoutUrl;
  validProviderWithoutUrl["providerId"] = "kimi";
  validProviderWithoutUrl["profileId"] = "default";
  stored.append(validProviderWithoutUrl);

  SettingsManager::instance()->setValue("workspace/openTabs", stored);

  MainWindow window;
  window.show();
  QApplication::processEvents();

  BrowserWorkspace* workspace = window.workspaceForTest();
  QCOMPARE(workspace->tabCount(), 1);
  BrowserTab* tab = workspace->currentTab();
  QVERIFY(tab != nullptr);
  QCOMPARE(tab->providerId(), QString("kimi"));
  QCOMPARE(tab->url(), ProviderRegistry::instance()->providerById("kimi")->url());
  window.close();
  QApplication::processEvents();
}

void TestSessionRestore::testFutureSchemaIsNotOverwritten() {
  QVariantList stored;
  QVariantMap entry;
  entry["providerId"] = "deepseek";
  entry["profileId"] = "default";
  entry["url"] = "https://chat.deepseek.com/";
  stored.append(entry);
  SettingsManager::instance()->setValue("workspace/openTabs", stored);
  SettingsManager::instance()->setValue("workspace/currentTabIndex", 0);
  SettingsManager::instance()->setValue("workspace/sessionVersion", 99);

  {
    MainWindow window;
    window.show();
    QApplication::processEvents();
    QCOMPARE(window.workspaceForTest()->tabCount(), 0);
  }

  QCOMPARE(TabStore::savedVersion(), 99);
  QCOMPARE(SettingsManager::instance()->value("workspace/openTabs").toList().size(), 1);
}

void TestSessionRestore::testDeletedProfileFallsBack() {
  ProfileManager* profiles = ProfileManager::instance();
  Profile* qwenWork = profiles->getOrCreateProfile("qwen", "work");
  QVERIFY(qwenWork != nullptr);
  QVERIFY(profiles->deleteProfile(qwenWork));

  QList<TabMetadata> tabs;
  tabs.append(TabMetadata("qwen", "work", QUrl("https://chat.qwen.ai/")));
  TabStore::save(tabs, 0);

  {
    MainWindow window;
    window.show();
    QApplication::processEvents();

    BrowserWorkspace* workspace = window.workspaceForTest();
    QCOMPARE(workspace->tabCount(), 1);
    BrowserTab* tab = workspace->currentTab();
    QVERIFY(tab != nullptr);
    QCOMPARE(tab->providerId(), QString("qwen"));
    QCOMPARE(tab->profileId(), QString("default"));
    QVERIFY(profiles->profileById("qwen:default") != nullptr);
    QVERIFY(QDir(profiles->profileById("qwen:default")->path()).exists());

    window.close();
    QApplication::processEvents();
  }
}

void TestSessionRestore::testMissingProviderIsSkipped() {
  QList<TabMetadata> tabs;
  tabs.append(TabMetadata("deepseek", "default", QUrl("https://chat.deepseek.com/")));
  tabs.append(TabMetadata("retired-provider", "default", QUrl("https://retired.example/")));
  tabs.append(TabMetadata("claude", "default", QUrl("https://claude.ai/")));
  TabStore::save(tabs, 1);

  {
    MainWindow window;
    window.show();
    QApplication::processEvents();

    const QStringList ids = window.workspaceForTest()->tabIds();
    QCOMPARE(ids.size(), 2);
    QCOMPARE(window.workspaceForTest()->tabById(ids[0])->providerId(), QString("deepseek"));
    QCOMPARE(window.workspaceForTest()->tabById(ids[1])->providerId(), QString("claude"));
    QCOMPARE(window.workspaceForTest()->currentTabId(), ids[1]);
    window.close();
    QApplication::processEvents();
  }
}

void TestSessionRestore::testSessionRecordsContainNoCredentials() {
  QList<TabMetadata> tabs;
  tabs.append(TabMetadata("chatgpt", "work",
                          QUrl("https://chatgpt.com/c/abc?model=gpt-4")));
  TabStore::save(tabs, 0);

  const QVariantList stored =
      SettingsManager::instance()->value("workspace/openTabs").toList();
  QCOMPARE(stored.size(), 1);

  const QVariantMap record = stored.at(0).toMap();
  const QStringList allowed = {"providerId", "profileId", "url", "title", "lifecycleState"};
  for (auto it = record.begin(); it != record.end(); ++it) {
    QVERIFY2(allowed.contains(it.key()),
             qPrintable(QString("Unexpected session key: %1").arg(it.key())));
    const QString value = it.value().toString().toLower();
    QVERIFY(!value.contains("cookie"));
    QVERIFY(!value.contains("token"));
    QVERIFY(!value.contains("password"));
  }
}

void TestSessionRestore::testShutdownDuringLoadKeepsSession() {
  QList<TabMetadata> tabs;
  tabs.append(TabMetadata("deepseek", "default", QUrl("https://chat.deepseek.com/")));
  tabs.append(TabMetadata("qwen", "default", QUrl("https://chat.qwen.ai/")));
  TabStore::save(tabs, 0);

  {
    MainWindow window;
    window.show();
    QApplication::processEvents();

    // Closing while the active tab is still loading must persist the workspace.
    window.close();
    QApplication::processEvents();
  }

  const QList<TabMetadata> restored = storedTabs();
  QCOMPARE(restored.size(), 2);
  QCOMPARE(restored.at(0).providerId(), QString("deepseek"));
  QCOMPARE(restored.at(1).providerId(), QString("qwen"));
}

int main(int argc, char* argv[]) {
  QApplication app(argc, argv);
  TestSessionRestore test;
  return QTest::qExec(&test, argc, argv);
}

#include "test_session_restore.moc"
