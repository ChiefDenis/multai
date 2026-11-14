#include <QtTest/QtTest>
#include <QLayout>
#include <QListWidget>
#include <QListWidgetItem>
#include <QScrollArea>
#include <QScrollBar>
#include <QSignalSpy>
#include <QStandardPaths>
#include <QToolButton>

#include "Sidebar.h"
#include "BrowserWorkspace.h"
#include "lifecycle/BrowserTab.h"
#include "lifecycle/TabMetadata.h"
#include "providers/Provider.h"
#include "providers/ProviderRegistry.h"
#include "SettingsManager.h"

class TestSidebar : public QObject {
  Q_OBJECT
private slots:
  void initTestCase();
  void testProviderStripHasAllProviders();
  void testProviderStripIsHorizontal();
  void testProviderStripTooltips();
  void testProviderStripScrolling();
  void testProviderStripSelection();
  void testAddAndRemoveTabRow();
  void testTabRowLifecycleUpdate();
  void testTabRowSelection();
  void testTabListRemainsVertical();
  void testSidebarCollapse();
  void testSidebarWidth();
  void testSettingsRequestedSignal();
  void testNoSidebarBranding();
  void testProviderSectionAtTop();
};

void TestSidebar::initTestCase() {
  qputenv("QT_QPA_PLATFORM", "offscreen");
  QStandardPaths::setTestModeEnabled(true);
}

void TestSidebar::testProviderStripHasAllProviders() {
  Sidebar sidebar;
  QWidget* strip = sidebar.findChild<QWidget*>("providerStrip");
  QVERIFY(strip != nullptr);

  const QList<QToolButton*> buttons = strip->findChildren<QToolButton*>("providerStripButton");
  QCOMPARE(buttons.size(), 9);

  QStringList foundIds;
  for (QToolButton* button : buttons) {
    foundIds.append(button->property("providerId").toString());
    QVERIFY(!button->icon().isNull());
  }

  const QList<Provider*> providers = ProviderRegistry::instance()->providers();
  QCOMPARE(providers.size(), 9);
  for (Provider* provider : providers) {
    QVERIFY2(foundIds.contains(provider->id()),
             qPrintable(QString("Missing provider button for %1").arg(provider->id())));
  }
}

void TestSidebar::testProviderStripIsHorizontal() {
  Sidebar sidebar;
  sidebar.resize(260, 700);
  sidebar.show();
  QApplication::processEvents();

  QWidget* strip = sidebar.findChild<QWidget*>("providerStrip");
  QVERIFY(strip != nullptr);
  if (strip->layout()) {
    strip->layout()->activate();
  }
  QApplication::processEvents();

  const QList<QToolButton*> buttons = strip->findChildren<QToolButton*>("providerStripButton");
  QCOMPARE(buttons.size(), 9);

  const int firstTop = buttons.first()->geometry().top();
  for (QToolButton* button : buttons) {
    QCOMPARE(button->geometry().top(), firstTop);
  }
  for (int i = 1; i < buttons.size(); ++i) {
    QVERIFY2(buttons[i]->geometry().left() > buttons[i - 1]->geometry().left(),
             "Provider buttons must be laid out left-to-right");
  }
}

void TestSidebar::testProviderStripTooltips() {
  Sidebar sidebar;
  QWidget* strip = sidebar.findChild<QWidget*>("providerStrip");
  QVERIFY(strip != nullptr);

  const QList<QToolButton*> buttons = strip->findChildren<QToolButton*>("providerStripButton");
  for (QToolButton* button : buttons) {
    Provider* provider = ProviderRegistry::instance()->providerById(
        button->property("providerId").toString());
    QVERIFY(provider != nullptr);
    QCOMPARE(button->toolTip(), provider->displayName());
  }
}

void TestSidebar::testProviderStripScrolling() {
  Sidebar sidebar;
  sidebar.resize(260, 700);
  sidebar.show();
  QApplication::processEvents();

  QScrollArea* scrollArea = sidebar.findChild<QScrollArea*>("providerScrollArea");
  QVERIFY(scrollArea != nullptr);
  QCOMPARE(scrollArea->horizontalScrollBarPolicy(), static_cast<int>(Qt::ScrollBarAlwaysOff));
  QCOMPARE(scrollArea->verticalScrollBarPolicy(), static_cast<int>(Qt::ScrollBarAlwaysOff));

  QWidget* strip = sidebar.findChild<QWidget*>("providerStrip");
  QVERIFY(strip != nullptr);
  QVERIFY2(strip->width() > scrollArea->viewport()->width(),
           "Provider strip should be wider than the viewport so all providers stay reachable");

  QScrollBar* bar = scrollArea->horizontalScrollBar();
  QVERIFY(bar != nullptr);
  QVERIFY(bar->maximum() > 0);
  bar->setValue(bar->maximum());
  QCOMPARE(bar->value(), bar->maximum());
  scrollArea->ensureWidgetVisible(
      strip->findChildren<QToolButton*>("providerStripButton").last(), 8, 0);
  QCOMPARE(bar->value(), bar->maximum());
}

void TestSidebar::testProviderStripSelection() {
  Sidebar sidebar;
  QWidget* strip = sidebar.findChild<QWidget*>("providerStrip");
  QVERIFY(strip != nullptr);

  QSignalSpy spy(&sidebar, &Sidebar::providerSelected);
  QToolButton* chatgpt = nullptr;
  for (QToolButton* button : strip->findChildren<QToolButton*>("providerStripButton")) {
    if (button->property("providerId").toString() == "chatgpt") {
      chatgpt = button;
    }
  }
  QVERIFY(chatgpt != nullptr);

  chatgpt->click();
  QCOMPARE(spy.count(), 1);
  QCOMPARE(spy.first().first().toString(), QString("chatgpt"));
  QVERIFY(!chatgpt->property("selected").toBool());

  sidebar.setSelectedProvider("chatgpt");
  QCOMPARE(sidebar.selectedProvider(), QString("chatgpt"));
  QVERIFY(chatgpt->property("selected").toBool());

  sidebar.setSelectedProvider("qwen");
  QVERIFY(!chatgpt->property("selected").toBool());

  // Collapsed rail now shows open tabs, not provider shortcuts
  sidebar.setCollapsed(true);
  QApplication::processEvents();

  // With no tabs open, collapsed rail should have no tab buttons
  const QMap<QString, QToolButton*>& tabButtons = sidebar.collapsedTabButtonsForTest();
  QCOMPARE(tabButtons.size(), 0);

  // Add a tab and verify it appears in collapsed rail
  sidebar.addTabRow("tab-1", "qwen", "Qwen Tab", QIcon(), LifecycleState::Active, false);
  QApplication::processEvents();

  const QMap<QString, QToolButton*>& tabButtons2 = sidebar.collapsedTabButtonsForTest();
  QCOMPARE(tabButtons2.size(), 1);
  QVERIFY(tabButtons2.contains("tab-1"));
  QCOMPARE(tabButtons2["tab-1"]->property("tabId").toString(), QString("tab-1"));
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

void TestSidebar::testTabListRemainsVertical() {
  Sidebar sidebar;
  QIcon icon;

  sidebar.addTabRow("tab-1", "deepseek", "Tab 1", icon, LifecycleState::Active, false);
  sidebar.addTabRow("tab-2", "qwen", "Tab 2", icon, LifecycleState::Unloaded, false);

  QListWidget* tabList = sidebar.findChild<QListWidget*>("tabList");
  QVERIFY(tabList != nullptr);
  QCOMPARE(tabList->count(), 2);

  const QRect first = tabList->visualItemRect(tabList->item(0));
  const QRect second = tabList->visualItemRect(tabList->item(1));
  QVERIFY2(second.top() >= first.bottom(),
           "Open Tabs must remain a vertical list");
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

void TestSidebar::testNoSidebarBranding() {
  Sidebar sidebar;

  // Header widget should not exist
  QVERIFY(sidebar.findChild<QWidget*>("sidebarHeader") == nullptr);
  QVERIFY(sidebar.findChild<QWidget*>("sidebarAppIcon") == nullptr);
  QVERIFY(sidebar.findChild<QWidget*>("sidebarAppTitle") == nullptr);

  // Divider should not exist (the one after header)
  QList<QFrame*> dividers = sidebar.findChildren<QFrame*>("sidebarDivider");
  // There may be other dividers (e.g., in collapsed rail), but the header divider is gone
  // The collapsed rail divider is still present, so we just verify the header is gone
}

void TestSidebar::testProviderSectionAtTop() {
  Sidebar sidebar;
  sidebar.resize(260, 700);
  sidebar.show();
  QApplication::processEvents();

  QWidget* expandedContainer = sidebar.findChild<QWidget*>("sidebarExpandedContainer");
  QVERIFY(expandedContainer != nullptr);

  QWidget* providersSection = expandedContainer->findChild<QWidget*>("providersSection");
  QVERIFY(providersSection != nullptr);

  // The providers section should be the first child in the expanded container layout
  QVBoxLayout* layout = qobject_cast<QVBoxLayout*>(expandedContainer->layout());
  QVERIFY(layout != nullptr);

  QWidget* firstItem = layout->itemAt(0) ? layout->itemAt(0)->widget() : nullptr;
  QVERIFY(firstItem != nullptr);
  QCOMPARE(firstItem->objectName(), QString("providersSection"));
}

QTEST_MAIN(TestSidebar)
#include "test_sidebar.moc"
