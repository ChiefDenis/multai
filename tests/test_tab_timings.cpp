#include <QtTest/QtTest>
#include <QStandardPaths>
#include <QElapsedTimer>
#include <QUrl>

#include "lifecycle/BrowserTab.h"
#include "lifecycle/LifecycleManager.h"
#include "SettingsManager.h"

class TestTabTimings : public QObject {
  Q_OBJECT
private slots:
  void initTestCase();
  void testTabCreationTime();
  void testTabActivationTimeActive();
  void testTabActivationTimeFrozen();
  void testTabActivationTimeDiscarded();
};

void TestTabTimings::initTestCase() {
  qputenv("QT_QPA_PLATFORM", "offscreen");
  QStandardPaths::setTestModeEnabled(true);
}

void TestTabTimings::testTabCreationTime() {
  QElapsedTimer timer;
  timer.start();

  BrowserTab tab("deepseek", "default", QUrl("https://www.deepseek.com/"),
                 LifecycleState::Unloaded);
  tab.activate();

  qint64 createTime = timer.elapsed();
  qDebug() << "Tab creation (activate):" << createTime << "ms";

  QVERIFY(tab.lifecycleState() == LifecycleState::Loading);
  QVERIFY(tab.pageCreated());
  QVERIFY(createTime < 5000);

  tab.closeTab();
}

void TestTabTimings::testTabActivationTimeActive() {
  BrowserTab tab1("deepseek", "default", QUrl("https://www.deepseek.com/"));
  BrowserTab tab2("qwen", "default", QUrl("https://qwen.ai/home"));

  tab1.activate();
  tab1.onLoadFinished(true);
  QCOMPARE(tab1.lifecycleState(), LifecycleState::Active);

  tab2.activate();
  tab2.onLoadFinished(true);
  QCOMPARE(tab2.lifecycleState(), LifecycleState::Active);

  QElapsedTimer timer;
  timer.start();

  tab1.activate();
  QCOMPARE(tab1.lifecycleState(), LifecycleState::Active);

  qint64 selectTime = timer.elapsed();
  qDebug() << "Active tab selection time:" << selectTime << "ms";

  QVERIFY(selectTime < 1000);

  tab1.closeTab();
  tab2.closeTab();
}

void TestTabTimings::testTabActivationTimeFrozen() {
  BrowserTab tab("claude", "default", QUrl("https://claude.ai/"));

  tab.activate();
  tab.onLoadFinished(true);
  QCOMPARE(tab.lifecycleState(), LifecycleState::Active);

  SettingsManager::instance()->setRespectRecommendedState(false);
  tab.freeze();
  QCOMPARE(tab.lifecycleState(), LifecycleState::Frozen);

  QElapsedTimer timer;
  timer.start();

  tab.activate();
  QCOMPARE(tab.lifecycleState(), LifecycleState::Active);

  qint64 activateTime = timer.elapsed();
  qDebug() << "Frozen tab activation time:" << activateTime << "ms";

  QVERIFY(activateTime < 1000);

  SettingsManager::instance()->setRespectRecommendedState(true);
  tab.closeTab();
}

void TestTabTimings::testTabActivationTimeDiscarded() {
  BrowserTab tab("perplexity", "default", QUrl("https://www.perplexity.ai/"));

  tab.activate();
  tab.onLoadFinished(true);
  QCOMPARE(tab.lifecycleState(), LifecycleState::Active);

  SettingsManager::instance()->setRespectRecommendedState(false);
  tab.hibernate();
  QCOMPARE(tab.lifecycleState(), LifecycleState::Discarded);

  QElapsedTimer timer;
  timer.start();

  tab.activate();
  QCOMPARE(tab.lifecycleState(), LifecycleState::Loading);

  qint64 activateTime = timer.elapsed();
  qDebug() << "Discarded tab activation time:" << activateTime << "ms";

  QVERIFY(activateTime < 5000);

  SettingsManager::instance()->setRespectRecommendedState(true);
  tab.closeTab();
}

QTEST_MAIN(TestTabTimings)
#include "test_tab_timings.moc"
