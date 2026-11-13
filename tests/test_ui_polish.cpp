#include <QtTest/QtTest>
#include <QApplication>
#include <QPalette>
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
  void testTooltipStylesheetLight();
  void testTooltipStylesheetDark();
  void testTooltipPaletteLight();
  void testTooltipPaletteDark();
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
  QVERIFY(sheet.contains("#sidebar"));
  QVERIFY(sheet.contains("QPushButton"));
  QVERIFY(sheet.contains("#f8f9fa"));
  QVERIFY(sheet.contains("#3584e4"));
}

void TestUiPolish::testDarkStyleSheet() {
  const QString sheet = Theme::styleSheetForMode(Theme::Mode::Dark);
  QVERIFY(!sheet.isEmpty());
  QVERIFY(sheet.contains("QMainWindow"));
  QVERIFY(sheet.contains("#sidebar"));
  QVERIFY(sheet.contains("QPushButton"));
  QVERIFY(sheet.contains("#161616"));
  QVERIFY(sheet.contains("#78aeed"));
}

void TestUiPolish::testSystemStyleSheet() {
  const QString sheet = Theme::styleSheetForMode(Theme::Mode::System);
  QVERIFY(!sheet.isEmpty());
  QVERIFY(sheet.contains("QMainWindow"));
}

void TestUiPolish::testThemeSwitchingAppliesStylesheet() {
  const QString lightSheet = Theme::styleSheetForMode(Theme::Mode::Light);
  const QString darkSheet = Theme::styleSheetForMode(Theme::Mode::Dark);
  QVERIFY(!lightSheet.isEmpty());
  QVERIFY(!darkSheet.isEmpty());
  QVERIFY(lightSheet != darkSheet);

  QVERIFY(lightSheet.contains("#f8f9fa"));
  QVERIFY(darkSheet.contains("#161616"));
  QVERIFY(lightSheet.contains("#sidebar"));
  QVERIFY(darkSheet.contains("#sidebar"));
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

void TestUiPolish::testTooltipStylesheetLight() {
  const QString sheet = Theme::styleSheetForMode(Theme::Mode::Light);
  QVERIFY(sheet.contains("QToolTip"));
  QVERIFY(sheet.contains("background-color: #ffffff"));
  QVERIFY(sheet.contains("color: #212529"));
  QVERIFY(sheet.contains("border: 1px solid #dee2e6"));
  QVERIFY(sheet.contains("border-radius: 6px"));
  QVERIFY(sheet.contains("padding: 4px 8px"));
}

void TestUiPolish::testTooltipStylesheetDark() {
  const QString sheet = Theme::styleSheetForMode(Theme::Mode::Dark);
  QVERIFY(sheet.contains("QToolTip"));
  QVERIFY(sheet.contains("background-color: #242424"));
  QVERIFY(sheet.contains("color: #e0e0e0"));
  QVERIFY(sheet.contains("border: 1px solid #333333"));
  QVERIFY(sheet.contains("border-radius: 6px"));
  QVERIFY(sheet.contains("padding: 4px 8px"));
}

void TestUiPolish::testTooltipPaletteLight() {
  Theme::applyTheme("light");
  QApplication::processEvents();
  const QPalette palette = qApp->palette();
  QCOMPARE(palette.color(QPalette::ToolTipBase), QColor("#ffffff"));
  QCOMPARE(palette.color(QPalette::ToolTipText), QColor("#212529"));
}

void TestUiPolish::testTooltipPaletteDark() {
  Theme::applyTheme("dark");
  QApplication::processEvents();
  const QPalette palette = qApp->palette();
  QCOMPARE(palette.color(QPalette::ToolTipBase), QColor("#242424"));
  QCOMPARE(palette.color(QPalette::ToolTipText), QColor("#e0e0e0"));
}

QTEST_MAIN(TestUiPolish)
#include "test_ui_polish.moc"
