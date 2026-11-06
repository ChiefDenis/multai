#include <QtTest/QtTest>
#include <QSignalSpy>
#include <QStandardPaths>
#include <QToolButton>

#include "BrowserWorkspace.h"
#include "lifecycle/BrowserTab.h"
#include "lifecycle/TabMetadata.h"

class TestEmptyState : public QObject {
  Q_OBJECT
private slots:
  void initTestCase();
  void testEmptyStateShowsWhenNoTabs();
  void testEmptyStateDisappearsWhenTabAdded();
  void testEmptyStateReappearsWhenLastTabClosed();
  void testEmptyStateProviderShortcuts();
  void testEmptyStateProviderSignal();
};

void TestEmptyState::initTestCase() {
  qputenv("QT_QPA_PLATFORM", "offscreen");
  QStandardPaths::setTestModeEnabled(true);
}

void TestEmptyState::testEmptyStateShowsWhenNoTabs() {
  BrowserWorkspace workspace;
  QVERIFY(!workspace.hasTabs());
  QCOMPARE(workspace.currentTab(), static_cast<BrowserTab*>(nullptr));
}

void TestEmptyState::testEmptyStateDisappearsWhenTabAdded() {
  BrowserWorkspace workspace;
  QSignalSpy spy(&workspace, &BrowserWorkspace::tabActivated);

  BrowserTab* tab = new BrowserTab("deepseek", "default",
                                   QUrl("https://chat.deepseek.com/"),
                                   LifecycleState::Unloaded);
  workspace.addTab(tab, "tab-1");

  QVERIFY(workspace.hasTabs());
  QCOMPARE(workspace.tabCount(), 1);
  QCOMPARE(workspace.currentTab(), tab);

  tab->closeTab();
  tab->deleteLater();
}

void TestEmptyState::testEmptyStateReappearsWhenLastTabClosed() {
  BrowserWorkspace workspace;

  BrowserTab* tab = new BrowserTab("deepseek", "default",
                                   QUrl("https://chat.deepseek.com/"),
                                   LifecycleState::Unloaded);
  workspace.addTab(tab, "tab-1");
  QVERIFY(workspace.hasTabs());

  workspace.removeTab(tab, "tab-1");
  QVERIFY(!workspace.hasTabs());
  QCOMPARE(workspace.currentTab(), static_cast<BrowserTab*>(nullptr));
  QCOMPARE(workspace.currentTabId(), QString());
}

void TestEmptyState::testEmptyStateProviderShortcuts() {
  BrowserWorkspace workspace;

  // The empty state should have provider shortcut buttons
  QList<QToolButton*> buttons = workspace.findChildren<QToolButton*>();
  qDebug() << "Found" << buttons.size() << "tool buttons";
  QList<QToolButton*> namedButtons = workspace.findChildren<QToolButton*>(
      QString("emptyStateProviderButton"));
  qDebug() << "Found" << namedButtons.size() << "named provider buttons";
  QVERIFY(namedButtons.size() == 9);
}

void TestEmptyState::testEmptyStateProviderSignal() {
  BrowserWorkspace workspace;
  QSignalSpy spy(&workspace, &BrowserWorkspace::emptyStateProviderRequested);

  QList<QToolButton*> buttons = workspace.findChildren<QToolButton*>(
      QString("emptyStateProviderButton"));
  QVERIFY(!buttons.isEmpty());

  // Click the first provider button
  QTest::mouseClick(buttons.first(), Qt::LeftButton, Qt::NoModifier);
  QCOMPARE(spy.count(), 1);
}

QTEST_MAIN(TestEmptyState)
#include "test_empty_state.moc"
