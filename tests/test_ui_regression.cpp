#include <QtTest/QtTest>
#include <QStandardPaths>
#include <QToolButton>
#include <QListWidget>
#include <QSplitter>

#include "MainWindow.h"
#include "BrowserWorkspace.h"
#include "Sidebar.h"
#include "Theme.h"
#include "SettingsManager.h"

class TestUIRegression : public QObject {
  Q_OBJECT
private slots:
  void initTestCase();
  void testDarkThemeWorkspaceBackground();
  void testLightThemeWorkspaceBackground();
  void testSidebarExpandedByDefault();
  void testSidebarAllNineProvidersVisible();
  void testSidebarCollapsibleWithoutHeightChange();
  void testWorkspaceHeaderHasIcons();
  void testEmptyStateHasProviderShortcuts();
  void testThemeSwitchingUpdatesTheme();
};

void TestUIRegression::initTestCase() {
  qputenv("QT_QPA_PLATFORM", "offscreen");
  QStandardPaths::setTestModeEnabled(true);
}

void TestUIRegression::testDarkThemeWorkspaceBackground() {
  Theme::applyTheme("dark");
  QApplication::processEvents();

  QString sheet = Theme::styleSheetForMode(Theme::Mode::Dark);
  // Dark theme stylesheet should contain dark background color
  QVERIFY(sheet.contains("#161616"));
  QVERIFY(sheet.contains("#browserWorkspace"));
}

void TestUIRegression::testLightThemeWorkspaceBackground() {
  Theme::applyTheme("light");
  QApplication::processEvents();

  QString sheet = Theme::styleSheetForMode(Theme::Mode::Light);
  QVERIFY(sheet.contains("#f8f9fa"));
  QVERIFY(sheet.contains("#browserWorkspace"));
}

void TestUIRegression::testSidebarExpandedByDefault() {
  Sidebar sidebar;
  QVERIFY(!sidebar.isCollapsed());
  QCOMPARE(sidebar.width(), 260);  // kDefaultSidebarWidth
}

void TestUIRegression::testSidebarAllNineProvidersVisible() {
  Sidebar sidebar;
  QListWidget* providerList = sidebar.findChild<QListWidget*>("providerList");
  QVERIFY(providerList != nullptr);
  QCOMPARE(providerList->count(), 9);
}

void TestUIRegression::testSidebarCollapsibleWithoutHeightChange() {
  Sidebar sidebar;
  sidebar.resize(260, 700);
  int initialHeight = sidebar.height();

  sidebar.setCollapsed(true);
  QVERIFY(sidebar.isCollapsed());
  QCOMPARE(sidebar.width(), 60);
  QCOMPARE(sidebar.height(), initialHeight);  // Height unchanged

  sidebar.setCollapsed(false);
  QCOMPARE(sidebar.width(), 260);
  QCOMPARE(sidebar.height(), initialHeight);  // Height unchanged
}

void TestUIRegression::testWorkspaceHeaderHasIcons() {
  MainWindow window;
  window.show();
  QApplication::processEvents();

  QWidget* header = window.workspaceHeaderForTest();
  QVERIFY(header != nullptr);

  QList<QToolButton*> buttons = header->findChildren<QToolButton*>();
  QCOMPARE(buttons.size(), 4);  // Back, Forward, Reload, Open External

  for (QToolButton* btn : buttons) {
    QVERIFY(!btn->icon().isNull());  // Each button has an icon
    QVERIFY(!btn->toolTip().isEmpty());  // Each button has a tooltip
  }
}

void TestUIRegression::testEmptyStateHasProviderShortcuts() {
  BrowserWorkspace workspace;
  QList<QToolButton*> buttons = workspace.findChildren<QToolButton*>(
      QString("emptyStateProviderButton"));
  QCOMPARE(buttons.size(), 9);

  for (QToolButton* btn : buttons) {
    QVERIFY(!btn->icon().isNull());
    QVERIFY(!btn->text().isEmpty());
  }
}

void TestUIRegression::testThemeSwitchingUpdatesTheme() {
  Theme::applyTheme("dark");
  QApplication::processEvents();

  QString sheet = Theme::styleSheetForMode(Theme::Mode::Dark);
  QVERIFY(sheet.contains("#161616"));

  Theme::applyTheme("light");
  QApplication::processEvents();

  sheet = Theme::styleSheetForMode(Theme::Mode::Light);
  QVERIFY(sheet.contains("#f8f9fa"));
}

QTEST_MAIN(TestUIRegression)
#include "test_ui_regression.moc"
