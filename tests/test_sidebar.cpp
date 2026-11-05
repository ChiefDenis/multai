#include <QtTest/QtTest>
#include <QListWidget>
#include <QListWidgetItem>
#include <QSignalSpy>
#include <QStandardPaths>
#include <QToolButton>

#include "Sidebar.h"
#include "BrowserWorkspace.h"
#include "lifecycle/BrowserTab.h"
#include "lifecycle/TabMetadata.h"
#include "providers/ProviderRegistry.h"
#include "SettingsManager.h"

class TestSidebar : public QObject {
  Q_OBJECT
private slots:
  void initTestCase();
  void testProviderRowsAdded();
  void testAllNineProvidersPresent();
  void testAddAndRemoveTabRow();
  void testTabRowLifecycleUpdate();
  void testTabRowSelection();
  void testSidebarCollapse();
  void testSidebarWidth();
  void testSettingsRequestedSignal();
};

void TestSidebar::initTestCase() {
  qputenv("QT_QPA_PLATFORM", "offscreen");
  QStandardPaths::setTestModeEnabled(true);
}

void TestSidebar::testProviderRowsAdded() {
  Sidebar sidebar;
  QListWidget* providerList = sidebar.findChild<QListWidget*>("providerList");
  QVERIFY(providerList != nullptr);
  QCOMPARE(providerList->count(), 9);
}

void TestSidebar::testAllNineProvidersPresent() {
  Sidebar sidebar;
  QListWidget* providerList = sidebar.findChild<QListWidget*>("providerList");
  QCOMPARE(providerList->count(), 9);

  QStringList expectedIds = {
      "deepseek", "qwen", "glm", "copilot", "gemini",
      "claude", "perplexity", "chatgpt", "kimi"};
  for (int i = 0; i < providerList->count(); ++i) {
    QListWidgetItem* item = providerList->item(i);
    QString providerId = item->data(Qt::UserRole).toString();
    QVERIFY(expectedIds.contains(providerId));
  }
}

void TestSidebar::testAddAndRemoveTabRow() {
  Sidebar sidebar;
  QIcon icon;

  sidebar.addTabRow("tab-1", "deepseek", "DeepSeek Tab", icon,
                    LifecycleState::Unloaded, false);

  QListWidget* tabList = sidebar.findChild<QListWidget*>("tabList");
  QCOMPARE(tabList->count(), 1);

  sidebar.removeTabRow("tab-1");
  QCOMPARE(tabList->count(), 0);
}

void TestSidebar::testTabRowLifecycleUpdate() {
  Sidebar sidebar;
  QIcon icon;

  sidebar.addTabRow("tab-1", "deepseek", "DeepSeek Tab", icon,
                    LifecycleState::Unloaded, false);

  sidebar.updateTabRow("tab-1", "DeepSeek Tab", LifecycleState::Active, false);
  QListWidgetItem* item = sidebar.findChild<QListWidget*>("tabList")->item(0);
  QVERIFY(item != nullptr);
  QVERIFY(item->toolTip().contains("Active"));

  sidebar.updateTabRow("tab-1", "DeepSeek Tab", LifecycleState::Frozen, false);
  item = sidebar.findChild<QListWidget*>("tabList")->item(0);
  QVERIFY(item->toolTip().contains("Frozen"));

  sidebar.updateTabRow("tab-1", "DeepSeek Tab", LifecycleState::Discarded, false);
  item = sidebar.findChild<QListWidget*>("tabList")->item(0);
  QVERIFY(item->toolTip().contains("Discarded"));

  sidebar.updateTabRow("tab-1", "DeepSeek Tab", LifecycleState::Unloaded, true);
  item = sidebar.findChild<QListWidget*>("tabList")->item(0);
  QVERIFY(item->toolTip().contains("Error"));
}

void TestSidebar::testTabRowSelection() {
  Sidebar sidebar;
  QIcon icon;

  sidebar.addTabRow("tab-1", "deepseek", "Tab 1", icon, LifecycleState::Active, false);
  sidebar.addTabRow("tab-2", "qwen", "Tab 2", icon, LifecycleState::Unloaded, false);

  sidebar.selectTabRow("tab-2");

  QListWidget* tabList = sidebar.findChild<QListWidget*>("tabList");
  QListWidgetItem* current = tabList->currentItem();
  QVERIFY(current != nullptr);
  QCOMPARE(current->data(Qt::UserRole).toString(), QString("tab-2"));
}

void TestSidebar::testSidebarCollapse() {
  Sidebar sidebar;
  QVERIFY(!sidebar.isCollapsed());

  sidebar.setCollapsed(true);
  QVERIFY(sidebar.isCollapsed());
  QCOMPARE(sidebar.width(), 60);

  sidebar.setCollapsed(false);
  QVERIFY(!sidebar.isCollapsed());
  QCOMPARE(sidebar.width(), 260);
}

void TestSidebar::testSidebarWidth() {
  Sidebar sidebar;
  sidebar.setSidebarWidth(300);
  QCOMPARE(sidebar.sidebarWidth(), 300);

  // Should clamp to valid range
  sidebar.setSidebarWidth(10);
  QCOMPARE(sidebar.sidebarWidth(), 60);

  sidebar.setSidebarWidth(500);
  QCOMPARE(sidebar.sidebarWidth(), 400);
}

void TestSidebar::testSettingsRequestedSignal() {
  Sidebar sidebar;
  QSignalSpy spy(&sidebar, &Sidebar::settingsRequested);

  QToolButton* btn = sidebar.findChild<QToolButton*>("sidebarSettingsButton");
  QVERIFY(btn != nullptr);
  QTest::mouseClick(btn, Qt::LeftButton, Qt::NoModifier);
  QCOMPARE(spy.count(), 1);
}

QTEST_MAIN(TestSidebar)
#include "test_sidebar.moc"
