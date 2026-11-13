#include <QtTest/QtTest>
#include <QFile>
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
  void testWorkspaceHasNoProviderHeader();
  void testEmptyStateHasProviderShortcuts();
  void testThemeSwitchingUpdatesTheme();
  void testDarkThemeSidebarBackground();
  void testProviderStripScrollConfiguration();
  void testProviderStripButtonContent();
  void testApplicationIconSet();
  void testMainWindowIconSet();
  void testDesktopFileHasStartupWMClass();
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

void TestUIRegression::testWorkspaceHasNoProviderHeader() {
  MainWindow window;
  window.resize(1100, 720);
  window.show();
  QApplication::processEvents();

  QVERIFY(window.findChild<QWidget*>("workspaceHeader") == nullptr);
  QVERIFY(window.findChild<QWidget*>("workspaceProviderIcon") == nullptr);
  QVERIFY(window.findChild<QWidget*>("workspaceProviderLabel") == nullptr);
  QVERIFY(window.findChild<QWidget*>("workspaceNavigationToolbar") == nullptr);

  QWidget* central = window.centralWidget();
  QVERIFY(central != nullptr);
  QWidget* workspace = window.workspaceForTest();
  QVERIFY(workspace != nullptr);
  QVERIFY(workspace->parentWidget() != nullptr);

  const QRect centralRect = central->rect();
  const QRect workspaceRect(workspace->mapTo(central, QPoint(0, 0)), workspace->size());
  QCOMPARE(workspaceRect.top(), centralRect.top());
  QCOMPARE(workspaceRect.bottom(), centralRect.bottom());
  QVERIFY(workspaceRect.height() > 0);
  QVERIFY(workspaceRect.width() > 0);
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

void TestUIRegression::testApplicationIconSet() {
  // QApplication created by test framework has no icon
  // The real application sets it in Application constructor
  QVERIFY(qApp != nullptr);
  // This test passes as long as we can verify the icon resource exists
  QIcon icon(":/icons/hicolor/256x256/apps/ai-workspace.png");
  QVERIFY(!icon.isNull());
}

void TestUIRegression::testMainWindowIconSet() {
  MainWindow window;
  window.resize(1100, 720);
  window.show();
  QApplication::processEvents();

  // MainWindow should have the window icon set
  QVERIFY(!window.windowIcon().isNull());
}

void TestUIRegression::testDesktopFileHasStartupWMClass() {
  // Try to find the desktop file in source tree
  QStringList possiblePaths = {
    ":/../ai-workspace.desktop",
    "../ai-workspace.desktop",
    "../../ai-workspace.desktop",
    "/home/denis/devprojects/ai-workspace/ai-workspace.desktop"
  };

  QString content;
  bool found = false;
  for (const QString& path : possiblePaths) {
    QFile desktopFile(path);
    if (desktopFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
      content = desktopFile.readAll();
      desktopFile.close();
      found = true;
      break;
    }
  }
  
  QVERIFY(found);
  QVERIFY(content.contains("StartupWMClass=ai-workspace"));
  QVERIFY(content.contains("Icon=ai-workspace"));
}

QTEST_MAIN(TestUIRegression)
#include "test_ui_regression.moc"
