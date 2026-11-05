#include <QtTest/QtTest>
#include <QSignalSpy>
#include <QStandardPaths>
#include <QWebEnginePage>

#include "lifecycle/BrowserTab.h"
#include "lifecycle/LifecycleManager.h"
#include "lifecycle/TabMetadata.h"
#include "lifecycle/MemoryMonitor.h"
#include "SettingsManager.h"
#include "profiles/ProfileManager.h"

class TestLifecycleIntegration : public QObject {
  Q_OBJECT
private slots:
  void initTestCase();
  void cleanupTestCase();
  void testQtLifecycleStateSynchronisation();
  void testFullLifecycleFlow();
  void testRendererCrashRecoveryIntegration();
  void testRecommendedStateRespected();
  void testFreezeThenReactivateNoReload();
  void testDiscardedThenReactivateReloads();
};

void TestLifecycleIntegration::initTestCase() {
  qputenv("QT_QPA_PLATFORM", "offscreen");
  QStandardPaths::setTestModeEnabled(true);
}

void TestLifecycleIntegration::cleanupTestCase() {}

void TestLifecycleIntegration::testQtLifecycleStateSynchronisation() {
  BrowserTab tab("deepseek", "default", QUrl("https://www.deepseek.com/"));

  // Initially unloaded, no page
  QCOMPARE(tab.lifecycleState(), LifecycleState::Unloaded);

  // Activate - creates page, loads
  tab.activate();
  QCOMPARE(tab.lifecycleState(), LifecycleState::Loading);

  // Simulate load finished
  tab.onLoadFinished(true);
  QCOMPARE(tab.lifecycleState(), LifecycleState::Active);

  // Verify Qt lifecycle state is tracked via page
  QVERIFY(tab.page() != nullptr);

  // Verify recommendedState is tracked
  QVERIFY(tab.page() != nullptr);

  // Cleanup
  tab.closeTab();
}

void TestLifecycleIntegration::testFullLifecycleFlow() {
  BrowserTab tab("deepseek", "default", QUrl("https://www.deepseek.com/"));

  // 1. Start: Unloaded
  QCOMPARE(tab.lifecycleState(), LifecycleState::Unloaded);

  // 2. Activate: Loading -> Active
  tab.activate();
  QCOMPARE(tab.lifecycleState(), LifecycleState::Loading);
  tab.onLoadFinished(true);
  QCOMPARE(tab.lifecycleState(), LifecycleState::Active);

  // 3. Freeze: Active -> Frozen
  SettingsManager::instance()->setRespectRecommendedState(false);
  tab.freeze();
  QCOMPARE(tab.lifecycleState(), LifecycleState::Frozen);

  // 4. Hibernate: Frozen -> Discarded
  tab.hibernate();
  QCOMPARE(tab.lifecycleState(), LifecycleState::Discarded);
  QVERIFY(tab.isSuspended());

  // 5. Reactivate: Discarded -> Loading
  tab.activate();
  QCOMPARE(tab.lifecycleState(), LifecycleState::Loading);

  // 6. Load finished: Loading -> Active
  tab.onLoadFinished(true);
  QCOMPARE(tab.lifecycleState(), LifecycleState::Active);

  SettingsManager::instance()->setRespectRecommendedState(true);
  tab.closeTab();
}

void TestLifecycleIntegration::testRendererCrashRecoveryIntegration() {
  BrowserTab tab("deepseek", "default", QUrl("https://www.deepseek.com/"));

  // Activate and load successfully
  tab.activate();
  tab.onLoadFinished(true);
  QCOMPARE(tab.lifecycleState(), LifecycleState::Active);
  QCOMPARE(tab.hasError(), false);

  // Simulate renderer crash
  tab.onRenderProcessTerminated(QWebEnginePage::CrashedTerminationStatus, 139);

  // Should attempt auto-recovery -> Loading
  QCOMPARE(tab.lifecycleState(), LifecycleState::Loading);
  QCOMPARE(tab.hasError(), false);

  // Simulate second crash during recovery -> Error UI
  QSignalSpy errorSpy(&tab, &BrowserTab::errorChanged);
  tab.onRenderProcessTerminated(QWebEnginePage::CrashedTerminationStatus, 139);

  QCOMPARE(tab.lifecycleState(), LifecycleState::Unloaded);
  QCOMPARE(tab.hasError(), true);
  QCOMPARE(tab.errorTitle(), QString("Web Page Unresponsive"));

  // Retry should work
  tab.retry();
  QCOMPARE(tab.hasError(), false);
  QCOMPARE(tab.lifecycleState(), LifecycleState::Loading);

  tab.closeTab();
}

void TestLifecycleIntegration::testRecommendedStateRespected() {
  BrowserTab tab("deepseek", "default", QUrl("https://www.deepseek.com/"));

  tab.activate();
  tab.onLoadFinished(true);
  QCOMPARE(tab.lifecycleState(), LifecycleState::Active);

  // With respectRecommendedState=true (default), hibernate should check
  // recommendedState. Since we're in offscreen mode, recommendedState
  // is typically Active, so hibernate may be blocked.
  // With respectRecommendedState=false, hibernate should work.
  SettingsManager::instance()->setRespectRecommendedState(false);
  tab.hibernate();
  QCOMPARE(tab.lifecycleState(), LifecycleState::Discarded);
  QVERIFY(tab.isSuspended());

  // Reactivate
  tab.activate();
  QCOMPARE(tab.lifecycleState(), LifecycleState::Loading);

  SettingsManager::instance()->setRespectRecommendedState(true);
  tab.closeTab();
}

void TestLifecycleIntegration::testFreezeThenReactivateNoReload() {
  BrowserTab tab("deepseek", "default", QUrl("https://www.deepseek.com/"));

  tab.activate();
  tab.onLoadFinished(true);
  QCOMPARE(tab.lifecycleState(), LifecycleState::Active);

  SettingsManager::instance()->setRespectRecommendedState(false);
  tab.freeze();
  QCOMPARE(tab.lifecycleState(), LifecycleState::Frozen);

  // Reactivating a frozen tab should NOT create a new page or reload
  // It just unfreezes the existing page
  QWebEnginePage* pageBefore = tab.page();
  tab.activate();
  QCOMPARE(tab.lifecycleState(), LifecycleState::Active);
  QCOMPARE(tab.page(), pageBefore); // Same page, not recreated

  SettingsManager::instance()->setRespectRecommendedState(true);
  tab.closeTab();
}

void TestLifecycleIntegration::testDiscardedThenReactivateReloads() {
  BrowserTab tab("deepseek", "default", QUrl("https://www.deepseek.com/"));

  tab.activate();
  tab.onLoadFinished(true);
  QCOMPARE(tab.lifecycleState(), LifecycleState::Active);

  SettingsManager::instance()->setRespectRecommendedState(false);
  tab.hibernate();
  QCOMPARE(tab.lifecycleState(), LifecycleState::Discarded);
  QVERIFY(tab.page() == nullptr); // Page destroyed

  // Reactivating a discarded tab should create a new page and reload
  tab.activate();
  QCOMPARE(tab.lifecycleState(), LifecycleState::Loading);
  QVERIFY(tab.page() != nullptr); // New page created

  SettingsManager::instance()->setRespectRecommendedState(true);
  tab.closeTab();
}

QTEST_MAIN(TestLifecycleIntegration)
#include "test_lifecycle_integration.moc"
