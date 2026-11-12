#include <QtTest/QtTest>

#include <QApplication>
#include <QImage>
#include <QLabel>
#include <QListWidget>
#include <QPalette>
#include <QPixmap>
#include <QStandardPaths>
#include <QToolButton>
#include <QWebEngineView>

#include "BrowserTab.h"
#include "BrowserWorkspace.h"
#include "MainWindow.h"
#include "ProfileManagementDialog.h"
#include "Provider.h"
#include "ProviderRegistry.h"
#include "SettingsManager.h"
#include "Sidebar.h"
#include "TabStore.h"
#include "Theme.h"
#include "ToolbarIcons.h"

namespace {

int countPixelsWithAlpha(const QPixmap& pixmap) {
  const QImage image = pixmap.toImage();
  int count = 0;
  for (int y = 0; y < image.height(); ++y) {
    for (int x = 0; x < image.width(); ++x) {
      if (qAlpha(image.pixel(x, y)) > 24) {
        ++count;
      }
    }
  }
  return count;
}

double lightPixelFraction(const QPixmap& pixmap, int threshold) {
  const QImage image = pixmap.toImage();
  int light = 0;
  int total = 0;
  for (int y = 0; y < image.height(); y += 2) {
    for (int x = 0; x < image.width(); x += 2) {
      const QColor colour = image.pixelColor(x, y);
      if (colour.lightness() > threshold) {
        ++light;
      }
      ++total;
    }
  }
  return total == 0 ? 0.0 : static_cast<double>(light) / total;
}

bool imagesDiffer(const QPixmap& first, const QPixmap& second) {
  return first.toImage() != second.toImage();
}

int maxIconLuminance(const QPixmap& pixmap) {
  const QImage image = pixmap.toImage();
  int maximum = 0;
  for (int y = 0; y < image.height(); ++y) {
    for (int x = 0; x < image.width(); ++x) {
      const QColor colour = image.pixelColor(x, y);
      if (colour.alpha() > 24) {
        maximum = qMax(maximum, colour.lightness());
      }
    }
  }
  return maximum;
}

}  // namespace

class TestVisualCorrections : public QObject {
  Q_OBJECT

private slots:
  void initTestCase();
  void init();

  void testWorkspaceHeaderShowsProviderIconAndName();
  void testToolbarIconsRender();
  void testSidebarLifecycleIndicatorsMatchStatus();
  void testLoadFailureClearsLoading();
  void testNavigationTimeoutClearsLoading();
  void testTabTitlesUseProviderNames();
  void testDarkSidebarRendersDark();
  void testDarkStatusBarTextContrast();
  void testEmptyStateIconContrast();
  void testCollapsedSidebarNavigation();
  void testCollapsedSidebarPreservesWindowSize();
  void testTabSelectionVisualState();
  void testAccountDialogVisibleAndCapturable();
  void testScreenshotStateConfiguration();
};

void TestVisualCorrections::initTestCase() {
  QStandardPaths::setTestModeEnabled(true);
}

void TestVisualCorrections::init() {
  TabStore::clear();
  SettingsManager::instance()->setTheme("light");
  Theme::applyTheme("light");
}

void TestVisualCorrections::testWorkspaceHeaderShowsProviderIconAndName() {
  SettingsManager::instance()->setTheme("light");
  Theme::applyTheme("light");

  MainWindow window;
  window.show();
  QApplication::processEvents();
  const QList<Provider*> providers = ProviderRegistry::instance()->providers();
  QCOMPARE(providers.size(), 9);

  for (Provider* provider : providers) {
    BrowserTab tab(provider->id(), "default", provider->url(), LifecycleState::Unloaded);
    window.updateWorkspaceHeaderForTest(&tab);

    QLabel* nameLabel = window.providerLabelForTest();
    QLabel* iconLabel = window.providerIconLabelForTest();
    QVERIFY(nameLabel != nullptr);
    QVERIFY(iconLabel != nullptr);

    QCOMPARE(nameLabel->text(), provider->displayName());
    QVERIFY2(nameLabel->pixmap().isNull(),
             "The provider name label must not be used as an icon container");
    QVERIFY2(!iconLabel->pixmap().isNull(),
             qPrintable(QString("Missing provider icon for %1").arg(provider->id())));
    QVERIFY(window.workspaceHeaderForTest()->isVisible());
  }
}

void TestVisualCorrections::testToolbarIconsRender() {
  SettingsManager::instance()->setTheme("light");
  Theme::applyTheme("light");

  MainWindow window;
  window.show();
  QApplication::processEvents();

  const QList<QPair<QToolButton*, QIcon>> icons{
      {window.backButtonForTest(), ToolbarIcons::backIcon(false)},
      {window.forwardButtonForTest(), ToolbarIcons::forwardIcon(false)},
      {window.reloadButtonForTest(), ToolbarIcons::reloadIcon(false)},
      {window.openExternalButtonForTest(), ToolbarIcons::openExternalIcon(false)},
  };

  for (const auto& entry : icons) {
    QToolButton* button = entry.first;
    QVERIFY(button != nullptr);
    QVERIFY(!button->icon().isNull());
    QVERIFY(!button->toolTip().isEmpty());

    const QPixmap normal = button->icon().pixmap(16, 16);
    const QPixmap disabled = button->icon().pixmap(16, 16, QIcon::Disabled);
    QVERIFY(!normal.isNull());
    QVERIFY(!disabled.isNull());
    QVERIFY(countPixelsWithAlpha(normal) > 0);
    QVERIFY(imagesDiffer(normal, disabled));

    const QPixmap rendered = button->grab();
    QVERIFY(!rendered.isNull());
    QVERIFY2(lightPixelFraction(rendered, 220) < 0.98,
             "Toolbar button rendered as a flat empty surface without an icon");
  }
  window.close();
  QApplication::processEvents();

  SettingsManager::instance()->setTheme("dark");
  Theme::applyTheme("dark");
  MainWindow darkWindow;
  darkWindow.show();
  QApplication::processEvents();

  for (QToolButton* button : {darkWindow.backButtonForTest(), darkWindow.forwardButtonForTest(),
                              darkWindow.reloadButtonForTest(),
                              darkWindow.openExternalButtonForTest()}) {
    QVERIFY(button != nullptr);
    button->setEnabled(true);
    QApplication::processEvents();
    const QPixmap rendered = button->grab();
    QVERIFY(maxIconLuminance(rendered) > 150);
    QVERIFY(lightPixelFraction(rendered, 220) < 0.98);
  }
}

void TestVisualCorrections::testSidebarLifecycleIndicatorsMatchStatus() {
  MainWindow window;
  BrowserTab tab("deepseek", "default", QUrl("https://chat.deepseek.com/"),
                 LifecycleState::Unloaded);

  Sidebar sidebar;
  sidebar.addTabRow("tab-1", "deepseek", "DeepSeek", QIcon(), LifecycleState::Unloaded, false);

  window.updateStatusDisplayForTest(&tab);
  QCOMPARE(window.statusBadgeLabelForTest()->text(), QString("Unloaded"));

  QListWidgetItem* item = sidebar.tabListForTest()->item(0);
  QVERIFY(item != nullptr);
  QVERIFY(item->toolTip().contains("Unloaded"));

  tab.onLoadStarted();
  sidebar.updateTabRow("tab-1", "DeepSeek", tab.lifecycleState(), false);
  window.updateStatusDisplayForTest(&tab);
  QCOMPARE(tab.lifecycleState(), LifecycleState::Loading);
  QCOMPARE(window.statusBadgeLabelForTest()->text(), QString("Loading..."));
  QVERIFY(item->toolTip().contains("Loading"));

  tab.onLoadFinished(false);
  sidebar.updateTabRow("tab-1", "DeepSeek", tab.lifecycleState(), tab.hasError());
  window.updateStatusDisplayForTest(&tab);
  QCOMPARE(window.statusBadgeLabelForTest()->text(), QString("Error"));
  QVERIFY(item->toolTip().contains("Error"));
  QVERIFY(!item->toolTip().contains("Loading"));
  QWidget* rowWidget = sidebar.tabListForTest()->itemWidget(item);
  QVERIFY(rowWidget != nullptr);
  QLabel* stateBadge = rowWidget->findChild<QLabel*>("tabStateBadge");
  QVERIFY(stateBadge != nullptr);
  QCOMPARE(stateBadge->text(), QString("!"));
}

void TestVisualCorrections::testLoadFailureClearsLoading() {
  BrowserTab tab("deepseek", "default", QUrl("https://chat.deepseek.com/"),
                 LifecycleState::Unloaded);

  tab.onLoadStarted();
  QCOMPARE(tab.lifecycleState(), LifecycleState::Loading);

  tab.onLoadFinished(false);
  QCOMPARE(tab.lifecycleState(), LifecycleState::Unloaded);
  QVERIFY(tab.hasError());
  QVERIFY(!tab.errorTitle().isEmpty());
  QVERIFY(tab.errorWidget() != nullptr);
}

void TestVisualCorrections::testNavigationTimeoutClearsLoading() {
  BrowserTab tab("deepseek", "default", QUrl("https://chat.deepseek.com/"),
                 LifecycleState::Unloaded);

  tab.onLoadStarted();
  QCOMPARE(tab.lifecycleState(), LifecycleState::Loading);

  const bool invoked = QMetaObject::invokeMethod(&tab, "onLoadTimeout", Qt::DirectConnection);
  QVERIFY(invoked);
  QCOMPARE(tab.lifecycleState(), LifecycleState::Unloaded);
  QVERIFY(tab.hasError());
  QVERIFY(tab.errorTitle().contains("Timed Out"));
}

void TestVisualCorrections::testTabTitlesUseProviderNames() {
  MainWindow window;
  BrowserTab tab("gemini", "default", QUrl("https://gemini.google.com/"),
                 LifecycleState::Active);

  QCOMPARE(window.tabDisplayTitleForTest(&tab), QString("Google Gemini"));

  tab.onTitleChanged("gemini.google.com");
  QVERIFY(tab.title().isEmpty());
  tab.onTitleChanged("https://gemini.google.com/app");
  QVERIFY(tab.title().isEmpty());

  tab.onTitleChanged("Google Gemini");
  QCOMPARE(tab.title(), QString("Google Gemini"));
  QCOMPARE(window.tabDisplayTitleForTest(&tab), QString("Google Gemini"));
}

void TestVisualCorrections::testDarkSidebarRendersDark() {
  SettingsManager::instance()->setTheme("dark");
  Theme::applyTheme("dark");

  MainWindow window;
  window.resize(1000, 700);
  window.show();
  QApplication::processEvents();

  Sidebar* sidebar = window.sidebarForTest();
  QVERIFY(sidebar != nullptr);
  const QPixmap rendered = sidebar->grab();
  QVERIFY(!rendered.isNull());
  QVERIFY2(lightPixelFraction(rendered, 180) < 0.10,
           "Dark sidebar contains large unintentional light surfaces");

  const QColor sidebarColour = rendered.toImage().pixelColor(rendered.width() / 2, 40);
  QVERIFY2(sidebarColour.lightness() < 80,
           qPrintable(QString("Dark sidebar surface is too light: %1")
                          .arg(sidebarColour.name())));
}

void TestVisualCorrections::testDarkStatusBarTextContrast() {
  SettingsManager::instance()->setTheme("dark");
  Theme::applyTheme("dark");

  MainWindow window;
  window.show();
  QApplication::processEvents();

  QLabel* statusLabel = window.statusTabInfoLabelForTest();
  QVERIFY(statusLabel != nullptr);
  statusLabel->ensurePolished();
  const QColor textColour = statusLabel->palette().color(QPalette::WindowText);
  QVERIFY2(textColour.lightness() > 120,
           qPrintable(QString("Status bar text is too dark in dark theme: %1")
                          .arg(textColour.name())));
}

void TestVisualCorrections::testEmptyStateIconContrast() {
  SettingsManager::instance()->setTheme("dark");
  Theme::applyTheme("dark");
  {
    BrowserWorkspace workspace;
    QLabel* label = workspace.emptyStateFolderIconLabelForTest();
    QVERIFY(label != nullptr);
    QVERIFY(!label->pixmap().isNull());
    QVERIFY2(maxIconLuminance(label->pixmap()) > 140,
             "Empty-state icon is not visible on the dark theme");
  }

  SettingsManager::instance()->setTheme("light");
  Theme::applyTheme("light");
  {
    BrowserWorkspace workspace;
    QLabel* label = workspace.emptyStateFolderIconLabelForTest();
    QVERIFY(label != nullptr);
    QVERIFY2(maxIconLuminance(label->pixmap()) < 100,
             "Empty-state icon is not visible on the light theme");
  }
}

void TestVisualCorrections::testCollapsedSidebarNavigation() {
  Sidebar sidebar;
  sidebar.resize(260, 700);
  sidebar.show();
  QApplication::processEvents();

  sidebar.setCollapsed(true);
  QVERIFY(sidebar.isCollapsed());
  QCOMPARE(sidebar.width(), 60);
  QVERIFY(sidebar.collapsedRailForTest()->isVisible());
  QVERIFY(!sidebar.providerListForTest()->isVisible());
  QVERIFY(!sidebar.tabListForTest()->isVisible());

  const QList<QToolButton*> providerButtons =
      sidebar.collapsedRailForTest()->findChildren<QToolButton*>("collapsedProviderButton");
  QCOMPARE(providerButtons.size(), 9);
  for (QToolButton* button : providerButtons) {
    QVERIFY(!button->toolTip().isEmpty());
    QVERIFY(!button->icon().isNull());
  }

  QSignalSpy providerSpy(&sidebar, &Sidebar::providerSelected);
  providerButtons.first()->click();
  QCOMPARE(providerSpy.count(), 1);
  QCOMPARE(providerSpy.first().first().toString(), providerButtons.first()->property("providerId").toString());

  QToolButton* tabsButton = sidebar.collapsedTabsButtonForTest();
  QVERIFY(tabsButton != nullptr);
  QVERIFY(tabsButton->isVisible());
  QVERIFY(!tabsButton->icon().isNull());
  QVERIFY(tabsButton->toolTip().contains("Open Tabs"));
}

void TestVisualCorrections::testCollapsedSidebarPreservesWindowSize() {
  MainWindow window;
  window.resize(1100, 720);
  window.show();
  QApplication::processEvents();

  const QSize before = window.size();
  window.collapseSidebarForTest();
  QApplication::processEvents();
  QCOMPARE(window.size(), before);

  window.expandSidebarForTest();
  QApplication::processEvents();
  QCOMPARE(window.size(), before);
}

void TestVisualCorrections::testTabSelectionVisualState() {
  SettingsManager::instance()->setTheme("dark");
  Theme::applyTheme("dark");

  Sidebar sidebar;
  sidebar.resize(260, 700);
  sidebar.addTabRow("tab-a", "deepseek", "DeepSeek", QIcon(), LifecycleState::Active, false);
  sidebar.addTabRow("tab-b", "qwen", "Qwen", QIcon(), LifecycleState::Active, false);
  sidebar.show();
  QApplication::processEvents();

  sidebar.selectTabRow("tab-a");
  QApplication::processEvents();

  QWidget* first = sidebar.tabListForTest()->itemWidget(sidebar.tabListForTest()->item(0));
  QWidget* second = sidebar.tabListForTest()->itemWidget(sidebar.tabListForTest()->item(1));
  QVERIFY(first != nullptr);
  QVERIFY(second != nullptr);
  QVERIFY(first->property("selected").toBool());
  QVERIFY(!second->property("selected").toBool());

  const QColor selectedColour = first->grab().toImage().pixelColor(6, 6);
  const QColor idleColour = second->grab().toImage().pixelColor(6, 6);
  QVERIFY2(selectedColour != idleColour,
           qPrintable(QString("Selected tab row is visually indistinguishable: %1")
                          .arg(selectedColour.name())));

  sidebar.selectTabRow("tab-b");
  QApplication::processEvents();
  QVERIFY(!first->property("selected").toBool());
  QVERIFY(second->property("selected").toBool());
}

void TestVisualCorrections::testAccountDialogVisibleAndCapturable() {
  SettingsManager::instance()->setTheme("dark");
  Theme::applyTheme("dark");
  {
    ProfileManagementDialog dialog;
    dialog.show();
    QVERIFY(dialog.isVisible());
    QApplication::processEvents();

    const QPixmap capture = dialog.grab();
    QVERIFY(!capture.isNull());
    QCOMPARE(capture.size(), dialog.size());
    QVERIFY(lightPixelFraction(capture, 0) > 0.0);
    const QImage image = capture.toImage();
    QVERIFY2(image.pixelColor(2, 2).lightness() < 80,
             qPrintable(QString("Account dialog surface is not dark: %1")
                            .arg(image.pixelColor(2, 2).name())));
  }

  SettingsManager::instance()->setTheme("light");
  Theme::applyTheme("light");
  {
    ProfileManagementDialog dialog;
    dialog.show();
    QVERIFY(dialog.isVisible());
    QApplication::processEvents();

    const QPixmap capture = dialog.grab();
    QVERIFY(!capture.isNull());
    QCOMPARE(capture.size(), dialog.size());
    const QImage image = capture.toImage();
    QVERIFY2(image.pixelColor(2, 2).lightness() > 180,
             qPrintable(QString("Account dialog surface is not light: %1")
                            .arg(image.pixelColor(2, 2).name())));
  }
}

void TestVisualCorrections::testScreenshotStateConfiguration() {
  MainWindow window;
  window.show();
  QApplication::processEvents();
  BrowserWorkspace* workspace = window.workspaceForTest();
  Sidebar* sidebar = window.sidebarForTest();
  QVERIFY(workspace != nullptr);
  QVERIFY(sidebar != nullptr);
  QCOMPARE(workspace->tabCount(), 0);
  QVERIFY(!sidebar->isCollapsed());

  window.collapseSidebarForTest();
  QVERIFY(sidebar->isCollapsed());
  QVERIFY(sidebar->collapsedRailForTest()->isVisible());

  window.expandSidebarForTest();
  QVERIFY(!sidebar->isCollapsed());
  QVERIFY(!sidebar->collapsedRailForTest()->isVisible());
}

int main(int argc, char* argv[]) {
  qputenv("QT_QPA_PLATFORM", "offscreen");
  QApplication app(argc, argv);
  TestVisualCorrections test;
  return QTest::qExec(&test, argc, argv);
}

#include "test_visual_corrections.moc"
