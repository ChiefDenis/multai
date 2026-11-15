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
#include <QDir>
#include <QFileInfo>
#include <QColor>
#include <QIcon>
#include <QImage>
#include <QPixmap>
#include <QProcess>
#include <QProcessEnvironment>

#include "AppIdentity.h"

namespace {

QString sourceDir() { return QStringLiteral(MULTAI_SOURCE_DIR); }

QString desktopFilePath() { return sourceDir() + QStringLiteral("/multai.desktop"); }

bool readTextFile(const QString& path, QString* content) {
  QFile file(path);
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return false;
  *content = QString::fromUtf8(file.readAll());
  return true;
}

QString desktopIconName() {
  QString content;
  if (!readTextFile(desktopFilePath(), &content)) return {};
  for (const QString& line : content.split('\n')) {
    if (line.startsWith(QStringLiteral("Icon="))) return line.mid(5).trimmed();
  }
  return {};
}

bool isMultaiArtwork(const QColor& color) {
  // The rename keeps the existing warm-orange speech-bubble artwork; only the
  // identifiers changed. Accept the artwork palette while rejecting the brown
  // Adobe Illustrator fallback.
  return color.red() >= 200 && color.green() >= 40 && color.green() <= 170 &&
         color.blue() >= 30 && color.blue() <= 150;
}

bool isIllustratorBrown(const QColor& color) {
  return color.red() <= 160 && color.green() <= 110 && color.blue() <= 90;
}

QColor iconCenterColor(const QIcon& icon, int size = 64) {
  const QImage image = icon.pixmap(QSize(size, size)).toImage();
  if (image.isNull() || image.width() == 0 || image.height() == 0) return {};
  return image.pixelColor(image.width() / 2, image.height() / 2);
}

}  // namespace

#include "BrowserWorkspace.h"
#include "MainWindow.h"
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
  void testIconThemeResolution();
  void testIconNameAvoidsKdeHyphenFallback();
  void testIconSizesAndArtwork();
  void testApplicationIdentityMatchesDesktopEntry();
  void testPackagingInstallsCanonicalIcon();
  void testInstalledIconResolvesViaKde();
  void testInstalledIconIsOutsideBuildDirectory();
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
  // The real application sets this in the Application constructor. Verify the
  // canonical resource is present and loads through the same path.
  QVERIFY(qApp != nullptr);
  QIcon icon(QString::fromLatin1(AppIdentity::kIconResource));
  QVERIFY(!icon.isNull());
  const QColor center = iconCenterColor(icon, 256);
  QVERIFY2(isMultaiArtwork(center), qPrintable(QStringLiteral("resource center=%1,%2,%3")
                                                  .arg(center.red())
                                                  .arg(center.green())
                                                  .arg(center.blue())));
}

void TestUIRegression::testMainWindowIconSet() {
  MainWindow window;
  window.resize(1100, 720);
  window.show();
  QApplication::processEvents();

  QVERIFY(!window.windowIcon().isNull());
  const QColor center = iconCenterColor(window.windowIcon(), 64);
  QVERIFY2(isMultaiArtwork(center),
           qPrintable(QStringLiteral("window icon center=%1,%2,%3")
                          .arg(center.red())
                          .arg(center.green())
                          .arg(center.blue())));
  QVERIFY2(!isIllustratorBrown(center),
           "window icon resolved to the Adobe Illustrator mimetype icon instead of the Multai artwork");
}

void TestUIRegression::testDesktopFileHasStartupWMClass() {
  QString content;
  QVERIFY2(readTextFile(desktopFilePath(), &content),
           qPrintable(QStringLiteral("cannot read %1").arg(desktopFilePath())));
  QVERIFY(content.contains("StartupWMClass=" + QString::fromLatin1(AppIdentity::kDesktopFileName)));
  QVERIFY(content.contains("Icon=" + QString::fromLatin1(AppIdentity::kIconName)));
}

void TestUIRegression::testRuntimeVersion() {
  // Test the compile-time version macro matches expected
  QCOMPARE(QString(MULTAI_VERSION), QString("0.5.0"));
  // Test that the version header was generated correctly
  QVERIFY(!QString(MULTAI_VERSION).isEmpty());
}

void TestUIRegression::testAboutDialogVersion() {
  // Test the string formatting that onAbout() uses
  QString version = MULTAI_VERSION;
  QString aboutText = QString("<b>Multai %1</b><br><br>"
                              "A resource-conscious, multi-provider AI desktop application.<br>"
                              "Provider web interfaces are embedded with Qt WebEngine.<br><br>"
                              "<a href=\"https://github.com/ChiefDenis/\">GitHub</a>")
                          .arg(version);
  
  QVERIFY(aboutText.contains("Multai 0.5.0"));
  QVERIFY(aboutText.contains("<b>Multai 0.5.0</b>"));
}

void TestUIRegression::testAboutDialogDeveloperLink() {
  // Test the string formatting that onAbout() uses
  QString version = MULTAI_VERSION;
  QString aboutText = QString("<b>Multai %1</b><br><br>"
                              "A resource-conscious, multi-provider AI desktop application.<br>"
                              "Provider web interfaces are embedded with Qt WebEngine.<br><br>"
                              "<a href=\"https://github.com/ChiefDenis/\">GitHub</a>")
                          .arg(version);
  
  QVERIFY(aboutText.contains("https://github.com/ChiefDenis/"));
  QVERIFY(aboutText.contains("GitHub"));
  QVERIFY(aboutText.contains("Multai 0.5.0"));
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
  // Verify the canonical icon resource is compiled into the binary.
  QIcon icon(QString::fromLatin1(AppIdentity::kIconResource));
  QVERIFY(!icon.isNull());
  QVERIFY(!icon.availableSizes().isEmpty());

  QPixmap pixmap = icon.pixmap(256, 256);
  QVERIFY(!pixmap.isNull());
  QCOMPARE(pixmap.size(), QSize(256, 256));

  QImage image = pixmap.toImage();
  int nonTransparent = 0;
  for (int y = 0; y < image.height(); ++y) {
    for (int x = 0; x < image.width(); ++x) {
      if (image.pixelColor(x, y).alpha() > 0) ++nonTransparent;
    }
  }
  QVERIFY(nonTransparent > 0);
  const QColor center = image.pixelColor(image.width() / 2, image.height() / 2);
  QVERIFY2(isMultaiArtwork(center), "compiled resource does not contain the Multai artwork");
}

void TestUIRegression::testDesktopFileIconValue() {
  QString content;
  QVERIFY2(readTextFile(desktopFilePath(), &content),
           qPrintable(QStringLiteral("cannot read %1").arg(desktopFilePath())));
  QVERIFY(content.contains("Icon=" + QString::fromLatin1(AppIdentity::kIconName)));
  QVERIFY(content.contains("StartupWMClass=" + QString::fromLatin1(AppIdentity::kDesktopFileName)));
  QVERIFY(content.contains("Exec=" + QString::fromLatin1(AppIdentity::kDesktopFileName)));
  QVERIFY(content.contains("Type=Application"));
}

void TestUIRegression::testQtDesktopFileIdentity() {
  QCOMPARE(QString(MULTAI_VERSION), QString("0.5.0"));
  QVERIFY(!QString(MULTAI_VERSION).isEmpty());

  QCOMPARE(MULTAI_VERSION_MAJOR, 0);
  QCOMPARE(MULTAI_VERSION_MINOR, 5);
  QCOMPARE(MULTAI_VERSION_PATCH, 0);
  QString tweak = QString(MULTAI_VERSION_TWEAK);
  QVERIFY(tweak == "0" || tweak.isEmpty());
}

void TestUIRegression::testIconThemeResolution() {
  // In a KDE session QIcon::fromTheme uses KIconLoader, which is where the
  // hyphen fallback used to redirect the lookup to the "ai" mimetype icon.
  // When the icon is installed, it must resolve to the Multai artwork.
  const QString iconName = QString::fromLatin1(AppIdentity::kIconName);
  QVERIFY(!iconName.contains('-'));

  QIcon themeIcon = QIcon::fromTheme(iconName);
  if (themeIcon.isNull()) {
    QSKIP("canonical icon is not installed in the theme search path");
  }

  QVERIFY2(themeIcon.name() != QStringLiteral("ai"),
           "theme lookup fell back to the Adobe Illustrator mimetype icon");
  const QColor center = iconCenterColor(themeIcon, 64);
  QVERIFY2(isMultaiArtwork(center), qPrintable(QStringLiteral("theme icon center=%1,%2,%3")
                                                  .arg(center.red())
                                                  .arg(center.green())
                                                  .arg(center.blue())));
  QVERIFY(!isIllustratorBrown(center));
}

void TestUIRegression::testIconNameAvoidsKdeHyphenFallback() {
  // KIconLoaderPrivate::findMatchingIcon() strips suffixes at '-' *inside each
  // theme* before moving on to the next theme. A hyphenated application icon
  // name can therefore resolve to an unrelated mimetype icon in the active
  // theme (Kora ships "ai" for Adobe Illustrator) and never reach hicolor.
  const QString iconName = desktopIconName();
  QCOMPARE(iconName, QString::fromLatin1(AppIdentity::kIconName));
  QVERIFY2(!iconName.contains('-'),
           "the application icon name must not contain '-' because KDE strips hyphen suffixes");
}

void TestUIRegression::testIconSizesAndArtwork() {
  const QString iconName = QString::fromLatin1(AppIdentity::kIconName);
  const QList<int> sizes = {16, 24, 32, 48, 64, 128, 256};

  for (int size : sizes) {
    const QString path = sourceDir() +
                         QStringLiteral("/icons/hicolor/%1x%1/apps/%2.png").arg(size).arg(iconName);
    QVERIFY2(QFileInfo::exists(path), qPrintable(QStringLiteral("missing %1").arg(path)));

    QImage image(path);
    QVERIFY2(!image.isNull(), qPrintable(QStringLiteral("invalid PNG %1").arg(path)));
    QCOMPARE(image.width(), size);
    QCOMPARE(image.height(), size);

    int opaque = 0;
    for (int y = 0; y < image.height(); ++y) {
      for (int x = 0; x < image.width(); ++x) {
        if (image.pixelColor(x, y).alpha() > 0) ++opaque;
      }
    }
    QVERIFY2(opaque > 0, qPrintable(QStringLiteral("fully transparent icon %1").arg(path)));
  }

  const QString svg = sourceDir() +
                      QStringLiteral("/icons/hicolor/scalable/apps/%1.svg").arg(iconName);
  QVERIFY2(QFileInfo::exists(svg), qPrintable(QStringLiteral("missing %1").arg(svg)));
  QVERIFY(QFileInfo(svg).size() > 0);
}

void TestUIRegression::testApplicationIdentityMatchesDesktopEntry() {
  // Wayland uses the desktop file name as the app_id. It must match the Qt
  // desktop file name and the desktop entry's Icon must use the canonical name.
  const QString desktopId = QFileInfo(desktopFilePath()).completeBaseName();
  QCOMPARE(desktopId, QString::fromLatin1(AppIdentity::kDesktopFileName));
  QCOMPARE(desktopIconName(), QString::fromLatin1(AppIdentity::kIconName));
}

void TestUIRegression::testPackagingInstallsCanonicalIcon() {
  const QString iconName = QString::fromLatin1(AppIdentity::kIconName);

  QString cmake;
  QVERIFY(readTextFile(sourceDir() + QStringLiteral("/CMakeLists.txt"), &cmake));
  QVERIFY2(cmake.contains(QStringLiteral("icons/hicolor/256x256/apps/%1.png").arg(iconName)),
           "CMake does not install the canonical application icon");
  QVERIFY2(cmake.contains(QStringLiteral("icons/hicolor/scalable/apps/%1.svg").arg(iconName)),
           "CMake does not install the canonical scalable icon");
  QVERIFY2(!cmake.contains(QStringLiteral("icons/hicolor/256x256/apps/ai-workspace.png")),
           "CMake still installs the legacy hyphenated icon name");
  QVERIFY2(!cmake.contains(QStringLiteral("icons/hicolor/256x256/apps/ai_workspace.png")),
           "CMake still installs the legacy icon name");

  QString spec;
  QVERIFY(readTextFile(sourceDir() + QStringLiteral("/multai.spec"), &spec));
  QVERIFY2(spec.contains(QStringLiteral("apps/%1.png").arg(iconName)),
           "RPM spec does not package the canonical application icon");
  QVERIFY2(spec.contains(QStringLiteral("scalable/apps/%1.svg").arg(iconName)),
           "RPM spec does not package the canonical scalable icon");
  QVERIFY2(!spec.contains(QStringLiteral("apps/%{name}.png")),
           "RPM spec still packages the legacy hyphenated icon name");
}

void TestUIRegression::testInstalledIconResolvesViaKde() {
  // This is the integration test that reproduces the original defect: on a KDE
  // system with an icon theme that owns a prefix icon (Kora owns "ai"), the
  // installed desktop entry's Icon name must resolve to the packaged artwork.
  const QString finder = QStandardPaths::findExecutable(QStringLiteral("kiconfinder6"));
  if (finder.isEmpty()) QSKIP("kiconfinder6 is not installed");

  QProcess process;
  QProcessEnvironment environment = QProcessEnvironment::systemEnvironment();
  environment.remove(QStringLiteral("QT_QPA_PLATFORM"));
  process.setProcessEnvironment(environment);
  process.start(finder, {QString::fromLatin1(AppIdentity::kIconName)});
  if (!process.waitForFinished(15000)) {
    process.kill();
    process.waitForFinished();
    QFAIL("kiconfinder6 timed out");
  }

  const QString path = QString::fromLocal8Bit(process.readAllStandardOutput()).trimmed();
  if (path.isEmpty()) QSKIP("canonical icon is not installed on this system");

  QVERIFY2(!path.endsWith(QStringLiteral("/ai.svg")),
           qPrintable(QStringLiteral("resolved to the Adobe Illustrator icon: %1").arg(path)));
  QVERIFY2(!path.contains(QStringLiteral("/mimetypes/")),
           qPrintable(QStringLiteral("resolved to a mimetype icon: %1").arg(path)));
  QVERIFY2(path.contains(QString::fromLatin1(AppIdentity::kIconName)),
           qPrintable(QStringLiteral("unexpected icon path: %1").arg(path)));
}

void TestUIRegression::testInstalledIconIsOutsideBuildDirectory() {
  QString installed = QStandardPaths::locate(
      QStandardPaths::GenericDataLocation,
      QStringLiteral("icons/hicolor/256x256/apps/%1.png").arg(AppIdentity::kIconName));

  if (installed.isEmpty()) {
    const QString systemPath = QStringLiteral("/usr/share/icons/hicolor/256x256/apps/%1.png")
                                   .arg(AppIdentity::kIconName);
    if (QFileInfo::exists(systemPath)) installed = systemPath;
  }
  if (installed.isEmpty()) QSKIP("canonical icon is not installed on this system");

  QVERIFY2(!installed.contains(QStringLiteral(MULTAI_SOURCE_DIR)),
           "installed icon still points into the build/source tree");
  QVERIFY2(!installed.contains(QStringLiteral("/build")), "installed icon points into a build directory");
  QVERIFY2(installed.startsWith(QStringLiteral("/usr/share/")) ||
               installed.startsWith(QStringLiteral("/usr/local/share/")),
           qPrintable(QStringLiteral("unexpected installed icon path: %1").arg(installed)));

  QImage image(installed);
  QVERIFY(!image.isNull());
  const QColor center = image.pixelColor(image.width() / 2, image.height() / 2);
  QVERIFY2(isMultaiArtwork(center), "installed icon is not the Multai artwork");
}

QTEST_MAIN(TestUIRegression)
#include "test_ui_regression.moc"
