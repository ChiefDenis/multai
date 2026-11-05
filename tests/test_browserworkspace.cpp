#include <QtTest/QtTest>
#include <QSignalSpy>
#include <QStandardPaths>

#include "BrowserWorkspace.h"
#include "Sidebar.h"
#include "lifecycle/BrowserTab.h"
#include "lifecycle/TabMetadata.h"

class TestBrowserWorkspace : public QObject {
  Q_OBJECT
private slots:
  void initTestCase();
  void testAddAndActivateTab();
  void testRemoveTab();
  void testEmptyStateWhenNoTabs();
  void testCurrentTabTracking();
  void testTabActivatedSignal();
};

void TestBrowserWorkspace::initTestCase() {
  qputenv("QT_QPA_PLATFORM", "offscreen");
  QStandardPaths::setTestModeEnabled(true);
}

void TestBrowserWorkspace::testAddAndActivateTab() {
  BrowserWorkspace workspace;
  QVERIFY(!workspace.hasTabs());

  BrowserTab* tab = new BrowserTab("deepseek", "default", QUrl("https://www.deepseek.com/"),
                                   LifecycleState::Unloaded);
  workspace.addTab(tab, "tab-1");

  QVERIFY(workspace.hasTabs());
  QCOMPARE(workspace.tabCount(), 1);
  QCOMPARE(workspace.currentTab(), tab);
  QCOMPARE(workspace.currentTabId(), QString("tab-1"));
  QCOMPARE(workspace.tabById("tab-1"), tab);
}

void TestBrowserWorkspace::testRemoveTab() {
  BrowserWorkspace workspace;

  BrowserTab* tab1 = new BrowserTab("deepseek", "default", QUrl("https://www.deepseek.com/"),
                                    LifecycleState::Unloaded);
  BrowserTab* tab2 = new BrowserTab("qwen", "default", QUrl("https://qwen.ai/home"),
                                    LifecycleState::Unloaded);

  workspace.addTab(tab1, "tab-1");
  workspace.addTab(tab2, "tab-2");
  QCOMPARE(workspace.tabCount(), 2);

  workspace.removeTab(tab1, "tab-1");
  QCOMPARE(workspace.tabCount(), 1);
  QCOMPARE(workspace.currentTab(), tab2);
  QCOMPARE(workspace.tabById("tab-1"), static_cast<BrowserTab*>(nullptr));

  workspace.removeTab(tab2, "tab-2");
  QCOMPARE(workspace.tabCount(), 0);
  QVERIFY(!workspace.hasTabs());
  QCOMPARE(workspace.currentTab(), static_cast<BrowserTab*>(nullptr));
}

void TestBrowserWorkspace::testEmptyStateWhenNoTabs() {
  BrowserWorkspace workspace;
  QVERIFY(!workspace.hasTabs());
  QCOMPARE(workspace.currentTab(), static_cast<BrowserTab*>(nullptr));
}

void TestBrowserWorkspace::testCurrentTabTracking() {
  BrowserWorkspace workspace;

  BrowserTab* tab1 = new BrowserTab("deepseek", "default", QUrl("https://www.deepseek.com/"),
                                    LifecycleState::Unloaded);
  BrowserTab* tab2 = new BrowserTab("qwen", "default", QUrl("https://qwen.ai/home"),
                                    LifecycleState::Unloaded);

  workspace.addTab(tab1, "tab-1");
  QCOMPARE(workspace.currentTabId(), QString("tab-1"));

  workspace.addTab(tab2, "tab-2");
  // Adding second tab doesn't change current (first remains)
  QCOMPARE(workspace.currentTabId(), QString("tab-1"));

  workspace.activateTab("tab-2");
  QCOMPARE(workspace.currentTabId(), QString("tab-2"));
  QCOMPARE(workspace.currentTab(), tab2);
}

void TestBrowserWorkspace::testTabActivatedSignal() {
  BrowserWorkspace workspace;
  QSignalSpy spy(&workspace, &BrowserWorkspace::tabActivated);

  BrowserTab* tab = new BrowserTab("deepseek", "default", QUrl("https://www.deepseek.com/"),
                                   LifecycleState::Unloaded);
  workspace.addTab(tab, "tab-1");

  // Adding first tab activates it and emits signal
  QCOMPARE(spy.count(), 1);
  QList<QVariant> args = spy.takeFirst();
  QCOMPARE(args.at(0).value<BrowserTab*>(), tab);
  QCOMPARE(args.at(1).toString(), QString("tab-1"));

  // Activating same tab again doesn't emit
  workspace.activateTab("tab-1");
  QCOMPARE(spy.count(), 0);
}

QTEST_MAIN(TestBrowserWorkspace)
#include "test_browserworkspace.moc"
