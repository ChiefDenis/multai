#include <QtTest/QtTest>

#include <QApplication>
#include <QColor>
#include <QImage>
#include <QLabel>
#include <QList>
#include <QListWidget>
#include <QListWidgetItem>
#include <QPixmap>
#include <QStandardPaths>
#include <QToolButton>
#include <QWidget>

#include "MainWindow.h"
#include "Provider.h"
#include "ProviderIcons.h"
#include "ProviderRegistry.h"
#include "SettingsManager.h"
#include "Sidebar.h"
#include "Theme.h"
#include "lifecycle/TabMetadata.h"

namespace {

const QStringList kProviderIds = {
    "deepseek", "qwen", "glm", "copilot", "gemini",
    "claude", "perplexity", "chatgpt", "kimi",
};

int opaquePixelCount(const QPixmap& pixmap) {
  const QImage image = pixmap.toImage();
  int count = 0;
  for (int y = 0; y < image.height(); ++y) {
    for (int x = 0; x < image.width(); ++x) {
      if (image.pixelColor(x, y).alpha() > 24) {
        ++count;
      }
    }
  }
  return count;
}

QRect opaqueBounds(const QPixmap& pixmap) {
  const QImage image = pixmap.toImage();
  QRect bounds;
  for (int y = 0; y < image.height(); ++y) {
    for (int x = 0; x < image.width(); ++x) {
      if (image.pixelColor(x, y).alpha() > 24) {
        bounds = bounds.united(QRect(x, y, 1, 1));
      }
    }
  }
  return bounds;
}

bool hasOpaquePixelLighterThan(const QPixmap& pixmap, int lightness) {
  const QImage image = pixmap.toImage();
  for (int y = 0; y < image.height(); ++y) {
    for (int x = 0; x < image.width(); ++x) {
      const QColor colour = image.pixelColor(x, y);
      if (colour.alpha() > 24 && colour.lightness() > lightness) {
        return true;
      }
    }
  }
  return false;
}

bool hasOpaquePixelDarkerThan(const QPixmap& pixmap, int lightness) {
  const QImage image = pixmap.toImage();
  for (int y = 0; y < image.height(); ++y) {
    for (int x = 0; x < image.width(); ++x) {
      const QColor colour = image.pixelColor(x, y);
      if (colour.alpha() > 24 && colour.lightness() < lightness) {
        return true;
      }
    }
  }
  return false;
}

}  // namespace

class TestProviderLogos : public QObject {
  Q_OBJECT

private slots:
  void initTestCase();
  void init();

  void testAllNineProvidersRenderAtRequiredSizes();
  void testLogosKeepTransparencyAndAspectRatio();
  void testLogosRenderInLightAndDarkThemes();
  void testProviderNavigationUsesRealLogos();
  void testSelectionAndLifecycleIndicatorsDoNotObscureLogos();
  void testThemeSwitchingRefreshesNavigationLogos();
};

void TestProviderLogos::initTestCase() {
  QStandardPaths::setTestModeEnabled(true);
}

void TestProviderLogos::init() {
  SettingsManager::instance()->setTheme("light");
  Theme::applyTheme("light");
}

void TestProviderLogos::testAllNineProvidersRenderAtRequiredSizes() {
  ProviderRegistry* registry = ProviderRegistry::instance();
  QCOMPARE(registry->providers().size(), kProviderIds.size());
  QCOMPARE(kProviderIds.size(), 9);

  for (const QString& providerId : kProviderIds) {
    QVERIFY2(registry->providerById(providerId) != nullptr,
             qPrintable(QString("Provider registry is missing %1").arg(providerId)));

    for (int size : {16, 20, 24, 32, 42}) {
      const QPixmap pixmap = providerPixmap(providerId, size);
      QVERIFY2(!pixmap.isNull(),
               qPrintable(QString("Logo missing for %1 at %2px").arg(providerId).arg(size)));
      QCOMPARE(pixmap.size(), QSize(size, size));
      QVERIFY2(opaquePixelCount(pixmap) > 8,
               qPrintable(QString("Logo blank for %1 at %2px").arg(providerId).arg(size)));
      QCOMPARE(pixmap.toImage().pixelColor(0, 0).alpha(), 0);
    }

    QVERIFY2(!registry->providerById(providerId)->icon().isNull(),
             qPrintable(QString("Provider icon is null for %1").arg(providerId)));
  }
}

void TestProviderLogos::testLogosKeepTransparencyAndAspectRatio() {
  constexpr int size = 42;
  constexpr int inset = size * 8 / 100;
  constexpr int content = size - 2 * inset;

  for (const QString& providerId : kProviderIds) {
    const QPixmap pixmap = providerPixmap(providerId, size);
    const QImage image = pixmap.toImage();
    QVERIFY(image.hasAlphaChannel());

    for (const QPoint& corner : {QPoint(0, 0), QPoint(size - 1, 0),
                                 QPoint(0, size - 1), QPoint(size - 1, size - 1)}) {
      QCOMPARE(image.pixelColor(corner).alpha(), 0);
    }

    const QRect bounds = opaqueBounds(pixmap);
    QVERIFY(!bounds.isEmpty());
    QVERIFY2(bounds.width() <= content + 1,
             qPrintable(QString("%1 is wider than its box").arg(providerId)));
    QVERIFY2(bounds.height() <= content + 1,
             qPrintable(QString("%1 is taller than its box").arg(providerId)));
  }

  const QRect copilotBounds = opaqueBounds(providerPixmap("copilot", size));
  QVERIFY2(copilotBounds.width() > copilotBounds.height(),
           "Copilot logo must retain its wider-than-tall aspect ratio");

  const QRect chatgptBounds = opaqueBounds(providerPixmap("chatgpt", size));
  QVERIFY(qAbs(chatgptBounds.width() - chatgptBounds.height()) <= 2);
}

void TestProviderLogos::testLogosRenderInLightAndDarkThemes() {
  for (const QString& providerId : kProviderIds) {
    SettingsManager::instance()->setTheme("light");
    Theme::applyTheme("light");
    const QPixmap light = providerPixmap(providerId, 64);
    QVERIFY(!light.isNull());

    SettingsManager::instance()->setTheme("dark");
    Theme::applyTheme("dark");
    const QPixmap dark = providerPixmap(providerId, 64);
    QVERIFY(!dark.isNull());

    if (providerId == "chatgpt" || providerId == "claude" || providerId == "kimi") {
      QVERIFY(hasOpaquePixelDarkerThan(light, 80));
      QVERIFY(hasOpaquePixelLighterThan(dark, 180));
      QVERIFY2(light.toImage() != dark.toImage(),
               qPrintable(QString("Monochrome logo did not follow theme for %1")
                              .arg(providerId)));
    } else {
      QVERIFY2(light.toImage() == dark.toImage(),
               qPrintable(QString("Brand-coloured logo was recoloured for %1")
                              .arg(providerId)));
      QVERIFY(opaquePixelCount(dark) > 8);
    }
  }

  SettingsManager::instance()->setTheme("light");
  Theme::applyTheme("light");
}

void TestProviderLogos::testProviderNavigationUsesRealLogos() {
  SettingsManager::instance()->setTheme("light");
  Theme::applyTheme("light");

  ProviderRegistry* registry = ProviderRegistry::instance();
  Sidebar sidebar;
  sidebar.resize(260, 640);
  sidebar.show();
  QApplication::processEvents();

  const QList<QToolButton*> stripButtons =
      sidebar.findChildren<QToolButton*>("providerStripButton");
  QCOMPARE(stripButtons.size(), 9);

  for (QToolButton* button : stripButtons) {
    const QString providerId = button->property("providerId").toString();
    Provider* provider = registry->providerById(providerId);
    QVERIFY(provider != nullptr);
    QVERIFY2(button->icon().pixmap(28, 28).toImage() ==
                 provider->icon().pixmap(28, 28).toImage(),
             qPrintable(QString("Provider strip icon mismatch for %1").arg(providerId)));
  }

  // Collapsed rail now shows open tabs, not permanent provider shortcuts
  sidebar.setCollapsed(true);
  QApplication::processEvents();

  // With no tabs, collapsed rail should be empty
  const QMap<QString, QToolButton*>& tabButtonsEmpty = sidebar.collapsedTabButtonsForTest();
  QCOMPARE(tabButtonsEmpty.size(), 0);

  // Add a tab and verify its provider logo appears in collapsed rail
  Provider* deepseek = registry->providerById("deepseek");
  QVERIFY(deepseek != nullptr);
  sidebar.addTabRow("tab-deepseek", "deepseek", "DeepSeek", deepseek->icon(),
                    LifecycleState::Unloaded, false);
  QApplication::processEvents();

  const QMap<QString, QToolButton*>& tabButtons = sidebar.collapsedTabButtonsForTest();
  QCOMPARE(tabButtons.size(), 1);
  QVERIFY(tabButtons.contains("tab-deepseek"));
  QToolButton* railButton = tabButtons["tab-deepseek"];
  QVERIFY2(railButton->icon().pixmap(26, 26).toImage() ==
               deepseek->icon().pixmap(26, 26).toImage(),
           "Collapsed rail tab icon does not use the provider logo");

  // Add another tab from a different provider
  Provider* qwen = registry->providerById("qwen");
  QVERIFY(qwen != nullptr);
  sidebar.addTabRow("tab-qwen", "qwen", "Qwen", qwen->icon(),
                    LifecycleState::Unloaded, false);
  QApplication::processEvents();

  const QMap<QString, QToolButton*>& tabButtons2 = sidebar.collapsedTabButtonsForTest();
  QCOMPARE(tabButtons2.size(), 2);
  QVERIFY(tabButtons2.contains("tab-qwen"));
  QToolButton* railButton2 = tabButtons2["tab-qwen"];
  QVERIFY2(railButton2->icon().pixmap(26, 26).toImage() ==
               qwen->icon().pixmap(26, 26).toImage(),
           "Collapsed rail tab icon does not use the provider logo");

  // Add a second tab from the same provider - should create separate icon
  sidebar.addTabRow("tab-deepseek-2", "deepseek", "DeepSeek 2", deepseek->icon(),
                    LifecycleState::Unloaded, false);
  QApplication::processEvents();

  const QMap<QString, QToolButton*>& tabButtons3 = sidebar.collapsedTabButtonsForTest();
  QCOMPARE(tabButtons3.size(), 3);
  QVERIFY(tabButtons3.contains("tab-deepseek-2"));
  QToolButton* railButton3 = tabButtons3["tab-deepseek-2"];
  QVERIFY2(railButton3->icon().pixmap(26, 26).toImage() ==
               deepseek->icon().pixmap(26, 26).toImage(),
           "Collapsed rail duplicate provider tab icon does not use the provider logo");

  QListWidgetItem* item = sidebar.tabListForTest()->item(0);
  QVERIFY(item != nullptr);
  QWidget* row = sidebar.tabListForTest()->itemWidget(item);
  QVERIFY(row != nullptr);
  QLabel* tabIcon = row->findChild<QLabel*>("tabIconLabel");
  QVERIFY(tabIcon != nullptr);
  QVERIFY2(tabIcon->pixmap().toImage() == deepseek->icon().pixmap(28, 28).toImage(),
           "Open Tabs row does not use the provider logo");
}

void TestProviderLogos::testSelectionAndLifecycleIndicatorsDoNotObscureLogos() {
  SettingsManager::instance()->setTheme("light");
  Theme::applyTheme("light");

  ProviderRegistry* registry = ProviderRegistry::instance();
  Sidebar sidebar;
  sidebar.resize(260, 640);
  sidebar.show();
  QApplication::processEvents();

  QToolButton* selected = nullptr;
  for (QToolButton* button : sidebar.findChildren<QToolButton*>("providerStripButton")) {
    if (button->property("providerId").toString() == "deepseek") {
      selected = button;
    }
  }
  QVERIFY(selected != nullptr);

  const QImage before = selected->icon().pixmap(28, 28).toImage();
  QVERIFY(opaquePixelCount(selected->icon().pixmap(28, 28)) > 8);

  sidebar.setSelectedProvider("deepseek");
  QApplication::processEvents();
  QVERIFY(selected->property("selected").toBool());
  QVERIFY2(selected->icon().pixmap(28, 28).toImage() == before,
           "Selection styling altered the provider logo art");
  QVERIFY(opaquePixelCount(selected->icon().pixmap(28, 28)) > 8);

  Provider* deepseek = registry->providerById("deepseek");
  QVERIFY(deepseek != nullptr);
  sidebar.addTabRow("tab-deepseek", "deepseek", "DeepSeek", deepseek->icon(),
                    LifecycleState::Loading, false);
  QApplication::processEvents();

  QListWidgetItem* item = sidebar.tabListForTest()->item(0);
  QVERIFY(item != nullptr);
  QWidget* row = sidebar.tabListForTest()->itemWidget(item);
  QVERIFY(row != nullptr);

  QLabel* iconLabel = row->findChild<QLabel*>("tabIconLabel");
  QLabel* stateBadge = row->findChild<QLabel*>("tabStateBadge");
  QVERIFY(iconLabel != nullptr);
  QVERIFY(stateBadge != nullptr);
  QCOMPARE(stateBadge->text(), QString("L"));
  QVERIFY2(!iconLabel->geometry().intersects(stateBadge->geometry()),
           "Lifecycle badge overlaps the provider logo");

  const QImage tabIconBefore = iconLabel->pixmap().toImage();
  sidebar.updateTabRow("tab-deepseek", "DeepSeek", LifecycleState::Discarded, false);
  QApplication::processEvents();
  QCOMPARE(stateBadge->text(), QString("H"));
  QVERIFY2(iconLabel->pixmap().toImage() == tabIconBefore,
           "Lifecycle state changes altered the provider logo art");
}

void TestProviderLogos::testThemeSwitchingRefreshesNavigationLogos() {
  SettingsManager::instance()->setTheme("light");
  Theme::applyTheme("light");

  MainWindow window;
  window.show();
  QApplication::processEvents();

  QToolButton* chatgpt = nullptr;
  for (QToolButton* button :
       window.sidebarForTest()->findChildren<QToolButton*>("providerStripButton")) {
    if (button->property("providerId").toString() == "chatgpt") {
      chatgpt = button;
    }
  }
  QVERIFY(chatgpt != nullptr);

  const QImage light = chatgpt->icon().pixmap(28, 28).toImage();
  QVERIFY(hasOpaquePixelDarkerThan(chatgpt->icon().pixmap(28, 28), 80));

  window.setThemeForTest("dark");
  QApplication::processEvents();

  const QImage dark = chatgpt->icon().pixmap(28, 28).toImage();
  QVERIFY2(light != dark, "Sidebar logo did not refresh after a theme change");
  QVERIFY2(hasOpaquePixelLighterThan(chatgpt->icon().pixmap(28, 28), 180),
           "Sidebar logo is not legible on the dark theme");

  window.setThemeForTest("light");
  QApplication::processEvents();
}

QTEST_MAIN(TestProviderLogos)
#include "test_provider_logos.moc"
