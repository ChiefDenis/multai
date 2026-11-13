#include <QtTest/QtTest>
#include <QStandardPaths>
#include <QToolButton>
#include <QListWidget>
#include <QListWidgetItem>
#include <QScrollArea>
#include <QSplitter>
#include <QWidget>
#include <Qt>

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
  void testWorkspaceHeaderHasNoNavigationToolbar();
  void testEmptyStateHasProviderShortcuts();
  void testThemeSwitchingUpdatesTheme();
  void testDarkThemeSidebarBackground();
  void testProviderStripScrollConfiguration();
  void testProviderStripButtonContent();
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
  QWidget* strip = sidebar.findChild<QWidget*>("providerStrip");
  QVERIFY(strip != nullptr);
  QCOMPARE(strip->findChildren<QToolButton*>("providerStripButton").size(), 9);
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

void TestUIRegression::testWorkspaceHeaderHasNoNavigationToolbar() {
  MainWindow window;
  window.show();
  QApplication::processEvents();

  QWidget* header = window.workspaceHeaderForTest();
  QVERIFY(header != nullptr);

  QCOMPARE(header->findChildren<QToolButton*>().size(), 0);
  QVERIFY(window.providerIconLabelForTest() != nullptr);
  QVERIFY(window.providerLabelForTest() != nullptr);
  QVERIFY(window.findChild<QWidget*>("workspaceNavigationToolbar") == nullptr);

  // The workspace must be the direct neighbour of the header, with no toolbar row between them.
  QWidget* central = window.centralWidget();
  QVERIFY(central != nullptr);
  QVERIFY(window.workspaceForTest() != nullptr);
  const QRect headerRect(header->mapTo(central, QPoint(0, 0)), header->size());
  const QRect workspaceRect(window.workspaceForTest()->mapTo(central, QPoint(0, 0)),
                            window.workspaceForTest()->size());
  QVERIFY2(workspaceRect.top() <= headerRect.bottom() + 2,
           "The browser workspace must begin directly beneath the header");
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

void TestUIRegression::testDarkThemeSidebarBackground() {
  Theme::applyTheme("dark");
  QApplication::processEvents();

  QString sheet = Theme::styleSheetForMode(Theme::Mode::Dark);
  QVERIFY(sheet.contains("background-color: #1e1e1e"));
  QVERIFY(sheet.contains("#sidebarBottomSection"));
  QVERIFY(sheet.contains("background-color: #242424"));
}

void TestUIRegression::testProviderStripScrollConfiguration() {
  Sidebar sidebar;
  QScrollArea* scrollArea = sidebar.findChild<QScrollArea*>("providerScrollArea");
  QVERIFY(scrollArea != nullptr);
  QCOMPARE(scrollArea->horizontalScrollBarPolicy(), static_cast<int>(Qt::ScrollBarAlwaysOff));
  QCOMPARE(scrollArea->verticalScrollBarPolicy(), static_cast<int>(Qt::ScrollBarAlwaysOff));
  QVERIFY(scrollArea->widgetResizable() == false);
}

void TestUIRegression::testProviderStripButtonContent() {
  Sidebar sidebar;
  QWidget* strip = sidebar.findChild<QWidget*>("providerStrip");
  QVERIFY(strip != nullptr);

  const QList<QToolButton*> buttons = strip->findChildren<QToolButton*>("providerStripButton");
  QCOMPARE(buttons.size(), 9);
  for (QToolButton* button : buttons) {
    QVERIFY(button->icon().isNull() == false);
    QVERIFY(button->toolTip().isEmpty() == false);
    QVERIFY(button->width() >= 40);
  }
}

QTEST_MAIN(TestUIRegression)
#include "test_ui_regression.moc"
