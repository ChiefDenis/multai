#include <QtTest/QtTest>
#include "lifecycle/TabMetadata.h"
#include <QUrl>

class TestLifecycle : public QObject {
  Q_OBJECT
private slots:
  void testLifecycleStateEnum();
  void testTabMetadataInitialState();
  void testTabMetadataSetters();
  void testLifecycleStateToString();
  void testLifecycleStateFromString();
  void testCanTransitionFromUnloaded();
  void testCanTransitionFromLoading();
  void testCanTransitionFromActive();
  void testCanTransitionFromFrozen();
  void testCanTransitionFromDiscarded();
  void testCanTransitionFromClosing();
  void testCanTransitionRejectsIdentity();
};

void TestLifecycle::testLifecycleStateEnum() {
  QCOMPARE(static_cast<int>(LifecycleState::Unloaded), 0);
  QCOMPARE(static_cast<int>(LifecycleState::Loading), 1);
  QCOMPARE(static_cast<int>(LifecycleState::Active), 2);
  QCOMPARE(static_cast<int>(LifecycleState::Frozen), 3);
  QCOMPARE(static_cast<int>(LifecycleState::Discarded), 4);
  QCOMPARE(static_cast<int>(LifecycleState::Closing), 5);
}

void TestLifecycle::testTabMetadataInitialState() {
  TabMetadata metadata;
  QCOMPARE(metadata.providerId(), QString());
  QCOMPARE(metadata.profileId(), QString());
  QCOMPARE(metadata.url(), QUrl());
  QCOMPARE(metadata.lifecycleState(), LifecycleState::Unloaded);
  QCOMPARE(metadata.title(), QString());
  QCOMPARE(metadata.lastActiveTime(), qint64(0));
}

void TestLifecycle::testTabMetadataSetters() {
  QUrl url("https://example.com");
  TabMetadata metadata("deepseek", "default", url);

  QCOMPARE(metadata.providerId(), QString("deepseek"));
  QCOMPARE(metadata.profileId(), QString("default"));
  QCOMPARE(metadata.url(), url);

  metadata.setLifecycleState(LifecycleState::Active);
  QCOMPARE(metadata.lifecycleState(), LifecycleState::Active);

  metadata.setTitle("Test Title");
  QCOMPARE(metadata.title(), QString("Test Title"));

  metadata.setLastActiveTime(12345);
  QCOMPARE(metadata.lastActiveTime(), qint64(12345));
}

void TestLifecycle::testLifecycleStateToString() {
  QCOMPARE(lifecycleStateToString(LifecycleState::Unloaded), QString("Unloaded"));
  QCOMPARE(lifecycleStateToString(LifecycleState::Loading), QString("Loading"));
  QCOMPARE(lifecycleStateToString(LifecycleState::Active), QString("Active"));
  QCOMPARE(lifecycleStateToString(LifecycleState::Frozen), QString("Frozen"));
  QCOMPARE(lifecycleStateToString(LifecycleState::Discarded), QString("Discarded"));
  QCOMPARE(lifecycleStateToString(LifecycleState::Closing), QString("Closing"));
}

void TestLifecycle::testLifecycleStateFromString() {
  QCOMPARE(lifecycleStateFromString("Unloaded"), LifecycleState::Unloaded);
  QCOMPARE(lifecycleStateFromString("Loading"), LifecycleState::Loading);
  QCOMPARE(lifecycleStateFromString("Active"), LifecycleState::Active);
  QCOMPARE(lifecycleStateFromString("Frozen"), LifecycleState::Frozen);
  QCOMPARE(lifecycleStateFromString("Discarded"), LifecycleState::Discarded);
  QCOMPARE(lifecycleStateFromString("Closing"), LifecycleState::Closing);
  QCOMPARE(lifecycleStateFromString("garbage"), LifecycleState::Unloaded);
}

void TestLifecycle::testCanTransitionFromUnloaded() {
  QVERIFY(canTransition(LifecycleState::Unloaded, LifecycleState::Loading));
  QVERIFY(canTransition(LifecycleState::Unloaded, LifecycleState::Closing));
  QVERIFY(!canTransition(LifecycleState::Unloaded, LifecycleState::Active));
  QVERIFY(!canTransition(LifecycleState::Unloaded, LifecycleState::Frozen));
  QVERIFY(!canTransition(LifecycleState::Unloaded, LifecycleState::Discarded));
}

void TestLifecycle::testCanTransitionFromLoading() {
  QVERIFY(canTransition(LifecycleState::Loading, LifecycleState::Active));
  QVERIFY(canTransition(LifecycleState::Loading, LifecycleState::Unloaded));
  QVERIFY(canTransition(LifecycleState::Loading, LifecycleState::Closing));
  QVERIFY(!canTransition(LifecycleState::Loading, LifecycleState::Frozen));
}

void TestLifecycle::testCanTransitionFromActive() {
  QVERIFY(canTransition(LifecycleState::Active, LifecycleState::Frozen));
  QVERIFY(canTransition(LifecycleState::Active, LifecycleState::Discarded));
  QVERIFY(canTransition(LifecycleState::Active, LifecycleState::Loading));
  QVERIFY(canTransition(LifecycleState::Active, LifecycleState::Closing));
  QVERIFY(!canTransition(LifecycleState::Active, LifecycleState::Unloaded));
}

void TestLifecycle::testCanTransitionFromFrozen() {
  QVERIFY(canTransition(LifecycleState::Frozen, LifecycleState::Active));
  QVERIFY(canTransition(LifecycleState::Frozen, LifecycleState::Discarded));
  QVERIFY(canTransition(LifecycleState::Frozen, LifecycleState::Closing));
  QVERIFY(!canTransition(LifecycleState::Frozen, LifecycleState::Loading));
}

void TestLifecycle::testCanTransitionFromDiscarded() {
  QVERIFY(canTransition(LifecycleState::Discarded, LifecycleState::Loading));
  QVERIFY(canTransition(LifecycleState::Discarded, LifecycleState::Unloaded));
  QVERIFY(canTransition(LifecycleState::Discarded, LifecycleState::Closing));
  QVERIFY(!canTransition(LifecycleState::Discarded, LifecycleState::Active));
}

void TestLifecycle::testCanTransitionFromClosing() {
  QVERIFY(!canTransition(LifecycleState::Closing, LifecycleState::Active));
  QVERIFY(!canTransition(LifecycleState::Closing, LifecycleState::Unloaded));
  QVERIFY(!canTransition(LifecycleState::Closing, LifecycleState::Loading));
}

void TestLifecycle::testCanTransitionRejectsIdentity() {
  QVERIFY(!canTransition(LifecycleState::Active, LifecycleState::Active));
  QVERIFY(!canTransition(LifecycleState::Loading, LifecycleState::Loading));
  QVERIFY(!canTransition(LifecycleState::Discarded, LifecycleState::Discarded));
}

QTEST_MAIN(TestLifecycle)
#include "test_lifecycle.moc"
