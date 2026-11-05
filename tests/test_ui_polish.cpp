#include <QtTest/QtTest>
#include "providers/ProviderIcons.h"
#include "ui/Theme.h"
#include "lifecycle/TabMetadata.h"

class TestUiPolish : public QObject {
  Q_OBJECT
private slots:
  void initTestCase();
  void testThemeModeFromString();
  void testThemeModeToString();
  void testLightStyleSheet();
  void testDarkStyleSheet();
  void testSystemStyleSheet();
  void testThemeSwitchingAppliesStylesheet();
  void testProviderIconsWithLifecycleStates();
  void testProviderIconWithErrorBadge();
  void testAllNineProvidersHaveIcons();
};

void TestUiPolish::initTestCase() {
  qputenv("QT_QPA_PLATFORM", "offscreen");
}

void TestUiPolish::testThemeModeFromString() {
  QCOMPARE(Theme::modeFromString("light"), Theme::Mode::Light);
  QCOMPARE(Theme::modeFromString("dark"), Theme::Mode::Dark);
  QCOMPARE(Theme::modeFromString("system"), Theme::Mode::System);
  QCOMPARE(Theme::modeFromString("unknown"), Theme::Mode::System);
}

void TestUiPolish::testThemeModeToString() {
  QCOMPARE(Theme::modeToString(Theme::Mode::Light), QString("light"));
  QCOMPARE(Theme::modeToString(Theme::Mode::Dark), QString("dark"));
  QCOMPARE(Theme::modeToString(Theme::Mode::System), QString("system"));
}

void TestUiPolish::testLightStyleSheet() {
  const QString sheet = Theme::styleSheetForMode(Theme::Mode::Light);
  QVERIFY(!sheet.isEmpty());
  QVERIFY(sheet.contains("QMainWindow"));
  QVERIFY(sheet.contains("QTabBar::tab"));
  QVERIFY(sheet.contains("QPushButton"));
  QVERIFY(sheet.contains("#f6f6f6"));
}

void TestUiPolish::testDarkStyleSheet() {
  const QString sheet = Theme::styleSheetForMode(Theme::Mode::Dark);
  QVERIFY(!sheet.isEmpty());
  QVERIFY(sheet.contains("QMainWindow"));
  QVERIFY(sheet.contains("QTabBar::tab"));
  QVERIFY(sheet.contains("QPushButton"));
  QVERIFY(sheet.contains("#242424"));
}

void TestUiPolish::testSystemStyleSheet() {
  const QString sheet = Theme::styleSheetForMode(Theme::Mode::System);
  QVERIFY(!sheet.isEmpty());
  QVERIFY(sheet.contains("QMainWindow"));
}

void TestUiPolish::testProviderIconsWithLifecycleStates() {
  QIcon activeIcon = providerIcon("deepseek", LifecycleState::Active, false);
  QVERIFY(!activeIcon.isNull());
  QCOMPARE(activeIcon.availableSizes().isEmpty(), false);

  QIcon frozenIcon = providerIcon("claude", LifecycleState::Frozen, false);
  QVERIFY(!frozenIcon.isNull());

  QIcon discardedIcon = providerIcon("chatgpt", LifecycleState::Discarded, false);
  QVERIFY(!discardedIcon.isNull());

  QIcon loadingIcon = providerIcon("gemini", LifecycleState::Loading, false);
  QVERIFY(!loadingIcon.isNull());
}

void TestUiPolish::testProviderIconWithErrorBadge() {
  QIcon errorIcon = providerIcon("kimi", LifecycleState::Unloaded, true);
  QVERIFY(!errorIcon.isNull());

  QPixmap pixmap = providerPixmap("kimi", 32, LifecycleState::Unloaded, true);
  QCOMPARE(pixmap.isNull(), false);
  QCOMPARE(pixmap.width(), 32);
  QCOMPARE(pixmap.height(), 32);
}

void TestUiPolish::testThemeSwitchingAppliesStylesheet() {
  const QString lightSheet = Theme::styleSheetForMode(Theme::Mode::Light);
  const QString darkSheet = Theme::styleSheetForMode(Theme::Mode::Dark);
  QVERIFY(!lightSheet.isEmpty());
  QVERIFY(!darkSheet.isEmpty());
  QVERIFY(lightSheet != darkSheet);

  // Verify theme-specific colours are present
  QVERIFY(lightSheet.contains("#f6f6f6"));
  QVERIFY(darkSheet.contains("#242424"));
  QVERIFY(lightSheet.contains("QTabBar::tab"));
  QVERIFY(darkSheet.contains("QTabBar::tab"));
}

void TestUiPolish::testAllNineProvidersHaveIcons() {
  const QStringList providerIds = {
      "deepseek", "qwen", "glm", "copilot", "gemini",
      "claude", "perplexity", "chatgpt", "kimi"};
  for (const QString& id : providerIds) {
    QIcon icon = providerIcon(id, LifecycleState::Active, false);
    QVERIFY(!icon.isNull());
    QVERIFY(!icon.availableSizes().isEmpty());
  }
}

QTEST_MAIN(TestUiPolish)
#include "test_ui_polish.moc"
