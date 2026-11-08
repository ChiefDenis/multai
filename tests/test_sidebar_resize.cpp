#include <QtTest/QtTest>
#include <QSignalSpy>
#include <QStandardPaths>

#include "Sidebar.h"
#include "BrowserWorkspace.h"
#include "lifecycle/BrowserTab.h"
#include "lifecycle/LifecycleManager.h"
#include "SettingsManager.h"

class TestSidebarResize : public QObject {
  Q_OBJECT
private slots:
  void initTestCase();
  void testSidebarCollapseDoesNotChangeWindowHeight();
  void testSidebarExpandRestoresWidth();
  void testSidebarCollapsedWidthIs60();
  void testSidebarExpandedWidthIs260();
};

void TestSidebarResize::initTestCase() {
  qputenv("QT_QPA_PLATFORM", "offscreen");
  QStandardPaths::setTestModeEnabled(true);
}

void TestSidebarResize::testSidebarCollapseDoesNotChangeWindowHeight() {
  Sidebar sidebar;
  sidebar.resize(260, 700);
  int initialHeight = sidebar.height();

  sidebar.setCollapsed(true);
  QVERIFY(sidebar.isCollapsed());
  QCOMPARE(sidebar.width(), 60);  // kCollapsedSidebarWidth
  QCOMPARE(sidebar.height(), initialHeight);  // Height unchanged

  sidebar.setCollapsed(false);
  QVERIFY(!sidebar.isCollapsed());
  QCOMPARE(sidebar.width(), 260);  // kDefaultSidebarWidth
  QCOMPARE(sidebar.height(), initialHeight);  // Height unchanged
}

void TestSidebarResize::testSidebarExpandRestoresWidth() {
  Sidebar sidebar;
  sidebar.setCollapsed(true);
  QCOMPARE(sidebar.width(), 60);

  sidebar.setCollapsed(false);
  QCOMPARE(sidebar.width(), 260);
}

void TestSidebarResize::testSidebarCollapsedWidthIs60() {
  Sidebar sidebar;
  sidebar.setCollapsed(true);
  QCOMPARE(sidebar.width(), 60);
  QVERIFY(sidebar.minimumWidth() <= 60);
  QVERIFY(sidebar.maximumWidth() >= 60);
}

void TestSidebarResize::testSidebarExpandedWidthIs260() {
  Sidebar sidebar;
  sidebar.setCollapsed(false);
  QCOMPARE(sidebar.width(), 260);
  QVERIFY(sidebar.minimumWidth() <= 260);
  QVERIFY(sidebar.maximumWidth() >= 260);
}

QTEST_MAIN(TestSidebarResize)
#include "test_sidebar_resize.moc"
