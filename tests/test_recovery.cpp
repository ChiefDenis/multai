#include <QtTest/QtTest>
#include <QSignalSpy>
#include <QStandardPaths>
#include "BrowserTab.h"
#include "SettingsManager.h"
#include "profiles/ProfileManager.h"

class TestRecovery : public QObject {
  Q_OBJECT
private slots:
  void initTestCase();
  void testInitialState();
  void testLoadFailureShowsError();
  void testRetryFromLoadFailure();
  void testSingleRendererCrashBoundedAutoRecovery();
  void testRepeatedCrashShowsErrorUi();
  void testRetryFromRepeatedCrash();
  void testCloseRequestedSignal();
};

void TestRecovery::initTestCase() {
  qputenv("QT_QPA_PLATFORM", "offscreen");
  QStandardPaths::setTestModeEnabled(true);
}

void TestRecovery::testInitialState() {
  BrowserTab tab("deepseek", "default", QUrl("https://www.deepseek.com/"));
  QCOMPARE(tab.hasError(), false);
  QVERIFY(tab.errorTitle().isEmpty());
  QVERIFY(tab.errorMessage().isEmpty());
  QCOMPARE(tab.lifecycleState(), LifecycleState::Unloaded);
}

void TestRecovery::testLoadFailureShowsError() {
  BrowserTab tab("deepseek", "default", QUrl("https://www.deepseek.com/"));
  tab.activate();
  QCOMPARE(tab.lifecycleState(), LifecycleState::Loading);

  QSignalSpy errorSpy(&tab, &BrowserTab::errorChanged);
  tab.onLoadFinished(false);

  QCOMPARE(tab.lifecycleState(), LifecycleState::Unloaded);
  QCOMPARE(tab.hasError(), true);
  QCOMPARE(tab.errorTitle(), QString("Unable to Load Page"));
  QVERIFY(tab.errorMessage().contains("deepseek.com"));
  QCOMPARE(errorSpy.count(), 1);
  QCOMPARE(errorSpy.takeFirst().at(0).toBool(), true);
}

void TestRecovery::testRetryFromLoadFailure() {
  BrowserTab tab("deepseek", "default", QUrl("https://www.deepseek.com/"));
  tab.activate();
  tab.onLoadFinished(false);
  QCOMPARE(tab.hasError(), true);

  QSignalSpy errorSpy(&tab, &BrowserTab::errorChanged);
  tab.retry();

  QCOMPARE(tab.hasError(), false);
  QCOMPARE(tab.lifecycleState(), LifecycleState::Loading);
  QCOMPARE(errorSpy.count(), 1);
  QCOMPARE(errorSpy.takeFirst().at(0).toBool(), false);
}

void TestRecovery::testSingleRendererCrashBoundedAutoRecovery() {
  BrowserTab tab("deepseek", "default", QUrl("https://www.deepseek.com/"));
  tab.activate();
  tab.onLoadFinished(true);
  QCOMPARE(tab.lifecycleState(), LifecycleState::Active);

  tab.onRenderProcessTerminated(QWebEnginePage::CrashedTerminationStatus, 139);

  QCOMPARE(tab.lifecycleState(), LifecycleState::Loading);
  QCOMPARE(tab.hasError(), false);
}

void TestRecovery::testRepeatedCrashShowsErrorUi() {
  BrowserTab tab("deepseek", "default", QUrl("https://www.deepseek.com/"));
  tab.activate();
  tab.onLoadFinished(true);
  QCOMPARE(tab.lifecycleState(), LifecycleState::Active);

  // First crash: auto-reload
  tab.onRenderProcessTerminated(QWebEnginePage::CrashedTerminationStatus, 139);
  QCOMPARE(tab.lifecycleState(), LifecycleState::Loading);
  QCOMPARE(tab.hasError(), false);

  // Second crash during reload cycle: bounded reload terminates and shows error UI
  QSignalSpy errorSpy(&tab, &BrowserTab::errorChanged);
  tab.onRenderProcessTerminated(QWebEnginePage::CrashedTerminationStatus, 139);

  QCOMPARE(tab.lifecycleState(), LifecycleState::Unloaded);
  QCOMPARE(tab.hasError(), true);
  QCOMPARE(tab.errorTitle(), QString("Web Page Unresponsive"));
  QVERIFY(tab.errorMessage().contains("crashed unexpectedly"));
  QVERIFY(tab.errorMessage().contains("139"));
  QCOMPARE(errorSpy.count(), 1);
  QCOMPARE(errorSpy.takeFirst().at(0).toBool(), true);
}

void TestRecovery::testRetryFromRepeatedCrash() {
  BrowserTab tab("deepseek", "default", QUrl("https://www.deepseek.com/"));
  tab.activate();
  tab.onLoadFinished(true);
  tab.onRenderProcessTerminated(QWebEnginePage::CrashedTerminationStatus, 139);
  tab.onRenderProcessTerminated(QWebEnginePage::CrashedTerminationStatus, 139);
  QCOMPARE(tab.hasError(), true);

  // Clicking retry clears error and re-enters loading
  tab.retry();
  QCOMPARE(tab.hasError(), false);
  QCOMPARE(tab.lifecycleState(), LifecycleState::Loading);

  // Recovery attempt flag is reset, so a new crash will attempt auto-recovery again
  tab.onRenderProcessTerminated(QWebEnginePage::CrashedTerminationStatus, 139);
  QCOMPARE(tab.lifecycleState(), LifecycleState::Loading);
  QCOMPARE(tab.hasError(), false);
}

void TestRecovery::testCloseRequestedSignal() {
  BrowserTab tab("deepseek", "default", QUrl("https://www.deepseek.com/"));
  tab.activate();
  tab.onLoadFinished(false);

  QSignalSpy closeSpy(&tab, &BrowserTab::closeRequested);
  emit tab.closeRequested();
  QCOMPARE(closeSpy.count(), 1);
}

QTEST_MAIN(TestRecovery)
#include "test_recovery.moc"
