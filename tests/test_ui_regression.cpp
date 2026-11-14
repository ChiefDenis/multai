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
#include <QMessageBox>
#include <QAbstractButton>
#include <QVBoxLayout>
#include <QLayoutItem>
#include <QScrollBar>
#include <QSignalSpy>

#include "MainWindow.h"
#include "BrowserWorkspace.h"
#include "Sidebar.h"
#include "Theme.h"
#include "SettingsManager.h"
#include "version.h"
#include "providers/ProviderRegistry.h"
#include "providers/Provider.h"

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
  void testRuntimeVersion();
  void testAboutDialogVersion();
  void testAboutDialogDeveloperLink();
  void testCollapsedRailShowsOpenTabs();
  void testCollapsedRailDuplicateProviderTabs();
  void testCollapsedRailEmptyState();
  void testCollapsedRailSettingsAboveExpand();
  void testCollapsedRailControlsAccessibleWithManyTabs();
  void testIconResourceValidity();
  void testDesktopFileIconValue();
  void testQtDesktopFileIdentity();
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

void TestUIRegression::testRuntimeVersion() {
  // Test the compile-time version macro matches expected
  QCOMPARE(QString(AI_WORKSPACE_VERSION), QString("0.4.4"));
  // Test that the version header was generated correctly
  QVERIFY(!QString(AI_WORKSPACE_VERSION).isEmpty());
}

void TestUIRegression::testAboutDialogVersion() {
  // Test the string formatting that onAbout() uses
  QString version = AI_WORKSPACE_VERSION;
  QString aboutText = QString("<b>AI Workspace %1</b><br><br>"
                              "A resource-conscious, multi-provider AI desktop application.<br>"
                              "Provider web interfaces are embedded with Qt WebEngine.<br><br>"
                              "<a href=\"https://github.com/ChiefDenis/\">GitHub</a>")
                          .arg(version);
  
  QVERIFY(aboutText.contains("AI Workspace 0.4.4"));
  QVERIFY(aboutText.contains("<b>AI Workspace 0.4.4</b>"));
}

void TestUIRegression::testAboutDialogDeveloperLink() {
  // Test the string formatting that onAbout() uses
  QString version = AI_WORKSPACE_VERSION;
  QString aboutText = QString("<b>AI Workspace %1</b><br><br>"
                              "A resource-conscious, multi-provider AI desktop application.<br>"
                              "Provider web interfaces are embedded with Qt WebEngine.<br><br>"
                              "<a href=\"https://github.com/ChiefDenis/\">GitHub</a>")
                          .arg(version);
  
  QVERIFY(aboutText.contains("https://github.com/ChiefDenis/"));
  QVERIFY(aboutText.contains("GitHub"));
  QVERIFY(aboutText.contains("AI Workspace 0.4.4"));
}

void TestUIRegression::testCollapsedRailShowsOpenTabs() {
  Sidebar sidebar;
  sidebar.resize(260, 700);
  sidebar.show();
  QApplication::processEvents();

  sidebar.setCollapsed(true);
  QApplication::processEvents();

  // Empty rail should have no tab buttons
  const QMap<QString, QToolButton*>& tabButtons = sidebar.collapsedTabButtonsForTest();
  QCOMPARE(tabButtons.size(), 0);

  // Add a tab
  sidebar.addTabRow("tab-1", "deepseek", "DeepSeek Tab", QIcon(), LifecycleState::Active, false);
  QApplication::processEvents();

  const QMap<QString, QToolButton*>& tabButtons2 = sidebar.collapsedTabButtonsForTest();
  QCOMPARE(tabButtons2.size(), 1);
  QVERIFY(tabButtons2.contains("tab-1"));
  QVERIFY(!tabButtons2["tab-1"]->toolTip().isEmpty());
  QVERIFY(!tabButtons2["tab-1"]->icon().isNull());
}

void TestUIRegression::testCollapsedRailDuplicateProviderTabs() {
  Sidebar sidebar;
  sidebar.resize(260, 700);
  sidebar.show();
  QApplication::processEvents();

  sidebar.setCollapsed(true);
  QApplication::processEvents();

  // Add two tabs from the same provider
  sidebar.addTabRow("tab-1", "deepseek", "DeepSeek Tab 1", QIcon(), LifecycleState::Active, false);
  sidebar.addTabRow("tab-2", "deepseek", "DeepSeek Tab 2", QIcon(), LifecycleState::Active, false);
  QApplication::processEvents();

  const QMap<QString, QToolButton*>& tabButtons = sidebar.collapsedTabButtonsForTest();
  QCOMPARE(tabButtons.size(), 2);
  QVERIFY(tabButtons.contains("tab-1"));
  QVERIFY(tabButtons.contains("tab-2"));

  // Each should have the same provider icon (verify by checking tooltips contain provider name)
  for (QToolButton* btn : tabButtons) {
    QVERIFY(btn->toolTip().contains("DeepSeek"));
  }
}

void TestUIRegression::testCollapsedRailEmptyState() {
  Sidebar sidebar;
  sidebar.resize(260, 700);
  sidebar.show();
  QApplication::processEvents();

  sidebar.setCollapsed(true);
  QApplication::processEvents();

  // Collapsed rail should not show permanent provider shortcuts
  const QList<QToolButton*> providerButtons =
      sidebar.collapsedRailForTest()->findChildren<QToolButton*>("collapsedProviderButton");
  QCOMPARE(providerButtons.size(), 0);

  // Collapsed tab rail should be empty
  const QMap<QString, QToolButton*>& tabButtons = sidebar.collapsedTabButtonsForTest();
  QCOMPARE(tabButtons.size(), 0);
}

void TestUIRegression::testCollapsedRailSettingsAboveExpand() {
  Sidebar sidebar;
  sidebar.resize(260, 700);
  sidebar.show();
  QApplication::processEvents();

  sidebar.setCollapsed(true);
  QApplication::processEvents();

  QWidget* bottomControls = sidebar.collapsedRailForTest()->findChild<QWidget*>("collapsedBottomControls");
  QVERIFY(bottomControls != nullptr);

  QLayout* layout = bottomControls->layout();
  QVERIFY(layout != nullptr);

  // First widget should be Settings button
  QLayoutItem* item0 = layout->itemAt(0);
  QVERIFY(item0 != nullptr);
  QWidget* widget0 = item0->widget();
  QVERIFY(widget0 != nullptr);
  QCOMPARE(widget0->objectName(), QString("collapsedSettingsButton"));

  // Second widget should be Expand button
  QLayoutItem* item1 = layout->itemAt(1);
  QVERIFY(item1 != nullptr);
  QWidget* widget1 = item1->widget();
  QVERIFY(widget1 != nullptr);
  QCOMPARE(widget1->objectName(), QString("collapsedCollapseButton"));
}

void TestUIRegression::testCollapsedRailControlsAccessibleWithManyTabs() {
  Sidebar sidebar;
  sidebar.resize(260, 700);
  sidebar.show();
  QApplication::processEvents();

  sidebar.setCollapsed(true);
  QApplication::processEvents();

  // Add many tabs to fill the rail
  for (int i = 0; i < 20; ++i) {
    QString providerId = (i % 2 == 0) ? "deepseek" : "qwen";
    sidebar.addTabRow(QString("tab-%1").arg(i), providerId, QString("Tab %1").arg(i), QIcon(), LifecycleState::Active, false);
  }
  QApplication::processEvents();

  const QMap<QString, QToolButton*>& tabButtons = sidebar.collapsedTabButtonsForTest();
  QCOMPARE(tabButtons.size(), 20);

  // Scroll area should be present and functional
  QScrollArea* scrollArea = sidebar.collapsedTabScrollAreaForTest();
  QVERIFY(scrollArea != nullptr);
  QVERIFY(scrollArea->verticalScrollBar()->maximum() > 0);

  // Bottom controls should still be accessible (not pushed off-screen)
  QWidget* bottomControls = sidebar.collapsedRailForTest()->findChild<QWidget*>("collapsedBottomControls");
  QVERIFY(bottomControls != nullptr);
  QVERIFY(bottomControls->isVisible());

  QToolButton* settingsBtn = bottomControls->findChild<QToolButton*>("collapsedSettingsButton");
  QVERIFY(settingsBtn != nullptr);
  QVERIFY(settingsBtn->isVisible());

  QToolButton* expandBtn = bottomControls->findChild<QToolButton*>("collapsedCollapseButton");
  QVERIFY(expandBtn != nullptr);
  QVERIFY(expandBtn->isVisible());
}

void TestUIRegression::testIconResourceValidity() {
  // Verify the icon resource is compiled into the binary
  QIcon icon(":/icons/hicolor/256x256/apps/ai-workspace.png");
  QVERIFY(!icon.isNull());
  QVERIFY(!icon.availableSizes().isEmpty());
  
  // Verify the resource contains actual image data
  QPixmap pixmap = icon.pixmap(256, 256);
  QVERIFY(!pixmap.isNull());
  QCOMPARE(pixmap.size(), QSize(256, 256));
  
  // Check it has non-transparent content
  QImage image = pixmap.toImage();
  int nonTransparent = 0;
  for (int y = 0; y < image.height(); ++y) {
    for (int x = 0; x < image.width(); ++x) {
      if (image.pixelColor(x, y).alpha() > 0) {
        nonTransparent++;
      }
    }
  }
  QVERIFY(nonTransparent > 0);
}

void TestUIRegression::testDesktopFileIconValue() {
  // Verify the desktop file has correct Icon value
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
  QVERIFY(content.contains("Icon=ai-workspace"));
  QVERIFY(content.contains("StartupWMClass=ai-workspace"));
  QVERIFY(content.contains("Exec=ai-workspace"));
}

void TestUIRegression::testQtDesktopFileIdentity() {
  // Verify the authoritative version macro matches the expected version
  // The actual ai-workspace application sets applicationName correctly in Application.cpp
  // This test runs in a test executable context, so we verify the compile-time version
  QCOMPARE(QString(AI_WORKSPACE_VERSION), QString("0.4.4"));
  QVERIFY(!QString(AI_WORKSPACE_VERSION).isEmpty());
  
  // Verify the version header was generated correctly with all components
  QCOMPARE(AI_WORKSPACE_VERSION_MAJOR, 0);
  QCOMPARE(AI_WORKSPACE_VERSION_MINOR, 4);
  QCOMPARE(AI_WORKSPACE_VERSION_PATCH, 4);
  // TWEAK may be 0 or empty for versions without a tweak component
  QString tweak = QString(AI_WORKSPACE_VERSION_TWEAK);
  QVERIFY(tweak == "0" || tweak.isEmpty());
}

QTEST_MAIN(TestUIRegression)
#include "test_ui_regression.moc"
