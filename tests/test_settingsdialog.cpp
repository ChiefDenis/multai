#include <QtTest/QtTest>
#include <QCheckBox>
#include <QComboBox>
#include <QLabel>
#include <QLineEdit>
#include <QSpinBox>
#include <QStandardPaths>
#include "ui/SettingsDialog.h"
#include "ui/Theme.h"
#include "SettingsManager.h"

class TestSettingsDialog : public QObject {
  Q_OBJECT
private slots:
  void initTestCase();
  void testLoadAppliesCurrentSettings();
  void testApplySavesSettings();
  void testCancelDoesNotSaveSettings();
  void testThemeSelection();
  void testLifecycleSettingsDefaults();
  void testMemoryPressureThresholds();
  void testContrastLightTheme();
  void testContrastDarkTheme();
  void testFormLabelsVisible();
  void testPlaceholderTextContrast();
  void testDisabledStateContrast();
  void testFocusIndicatorsVisible();
};

void TestSettingsDialog::initTestCase() {
  qputenv("QT_QPA_PLATFORM", "offscreen");
  QStandardPaths::setTestModeEnabled(true);
}

void TestSettingsDialog::testLoadAppliesCurrentSettings() {
  SettingsManager* settings = SettingsManager::instance();
  settings->setTheme("dark");
  settings->setFreezeDelayMs(10 * 60 * 1000);
  settings->setHibernateDelayMs(60 * 60 * 1000);
  settings->setLifecycleAutomationEnabled(false);
  settings->setMemoryPressureEnabled(true);
  settings->setMemoryPressureThresholdPercent(10);
  settings->setMemoryPressureRecoveryThresholdPercent(25);

  SettingsDialog dialog(nullptr);
  // Dialog calls load() in constructor, so settings should be reflected
  QComboBox* themeCombo = dialog.findChild<QComboBox*>("themeCombo");
  QVERIFY(themeCombo != nullptr);
  QCOMPARE(themeCombo->currentData().toString(), QString("dark"));
  QSpinBox* freezeSpin = dialog.findChild<QSpinBox*>("freezeSpin");
  QVERIFY(freezeSpin != nullptr);
  QCOMPARE(freezeSpin->value(), 10);
  QSpinBox* hibernateSpin = dialog.findChild<QSpinBox*>("hibernateSpin");
  QVERIFY(hibernateSpin != nullptr);
  QCOMPARE(hibernateSpin->value(), 60);

  // Reset settings to defaults for other tests
  settings->setTheme("system");
  settings->setFreezeDelayMs(5 * 60 * 1000);
  settings->setHibernateDelayMs(30 * 60 * 1000);
}

void TestSettingsDialog::testApplySavesSettings() {
  SettingsManager* settings = SettingsManager::instance();
  settings->setTheme("system");
  settings->setFreezeDelayMs(5 * 60 * 1000);
  settings->setHibernateDelayMs(30 * 60 * 1000);

  SettingsDialog dialog(nullptr);

  // Change values
  QComboBox* themeCombo = dialog.findChild<QComboBox*>("themeCombo");
  QVERIFY(themeCombo != nullptr);
  themeCombo->setCurrentIndex(themeCombo->findData("dark"));
  QSpinBox* freezeSpin = dialog.findChild<QSpinBox*>("freezeSpin");
  QVERIFY(freezeSpin != nullptr);
  freezeSpin->setValue(15);
  QSpinBox* hibernateSpin = dialog.findChild<QSpinBox*>("hibernateSpin");
  QVERIFY(hibernateSpin != nullptr);
  hibernateSpin->setValue(45);

  QMetaObject::invokeMethod(&dialog, "apply", Qt::DirectConnection);

  QCOMPARE(settings->theme(), QString("dark"));
  QCOMPARE(settings->freezeDelayMs(), 15 * 60 * 1000);
  QCOMPARE(settings->hibernateDelayMs(), 45 * 60 * 1000);

  // Reset
  settings->setTheme("system");
  settings->setFreezeDelayMs(5 * 60 * 1000);
  settings->setHibernateDelayMs(30 * 60 * 1000);
}

void TestSettingsDialog::testCancelDoesNotSaveSettings() {
  SettingsManager* settings = SettingsManager::instance();
  settings->setTheme("light");
  settings->setFreezeDelayMs(5 * 60 * 1000);

  SettingsDialog dialog(nullptr);

  // Change values
  QComboBox* themeCombo = dialog.findChild<QComboBox*>("themeCombo");
  QVERIFY(themeCombo != nullptr);
  themeCombo->setCurrentIndex(themeCombo->findData("dark"));
  QSpinBox* freezeSpin = dialog.findChild<QSpinBox*>("freezeSpin");
  QVERIFY(freezeSpin != nullptr);
  freezeSpin->setValue(99);

  QMetaObject::invokeMethod(&dialog, "reject", Qt::DirectConnection);

  // Settings should be unchanged
  QCOMPARE(settings->theme(), QString("light"));
  QCOMPARE(settings->freezeDelayMs(), 5 * 60 * 1000);

  // Reset
  settings->setTheme("system");
}

void TestSettingsDialog::testThemeSelection() {
  SettingsManager* settings = SettingsManager::instance();

  SettingsDialog dialog(nullptr);
  QComboBox* themeCombo = dialog.findChild<QComboBox*>("themeCombo");
  QVERIFY(themeCombo != nullptr);

  // Verify all three themes are available
  QCOMPARE(themeCombo->count(), 3);
  QCOMPARE(themeCombo->itemData(0).toString(), QString("system"));
  QCOMPARE(themeCombo->itemData(1).toString(), QString("light"));
  QCOMPARE(themeCombo->itemData(2).toString(), QString("dark"));

  settings->setTheme("system");
}

void TestSettingsDialog::testLifecycleSettingsDefaults() {
  SettingsManager* settings = SettingsManager::instance();
  settings->setLifecycleAutomationEnabled(true);
  settings->setFreezeDelayMs(5 * 60 * 1000);
  settings->setHibernateDelayMs(30 * 60 * 1000);
  settings->setRespectRecommendedState(true);
  settings->setMemoryPressureEnabled(true);
  settings->setMemoryPressureThresholdPercent(15);
  settings->setMemoryPressureRecoveryThresholdPercent(20);

  SettingsDialog dialog(nullptr);

  QCheckBox* automationCheck =
      dialog.findChild<QCheckBox*>("automationCheck");
  QVERIFY(automationCheck != nullptr);
  QSpinBox* freezeSpin = dialog.findChild<QSpinBox*>("freezeSpin");
  QVERIFY(freezeSpin != nullptr);
  QSpinBox* hibernateSpin = dialog.findChild<QSpinBox*>("hibernateSpin");
  QVERIFY(hibernateSpin != nullptr);
  QCheckBox* respectCheck =
      dialog.findChild<QCheckBox*>("respectRecommendedCheck");
  QVERIFY(respectCheck != nullptr);
  QCheckBox* pressureCheck =
      dialog.findChild<QCheckBox*>("memoryPressureCheck");
  QVERIFY(pressureCheck != nullptr);
  QSpinBox* thresholdSpin =
      dialog.findChild<QSpinBox*>("memoryThresholdSpin");
  QVERIFY(thresholdSpin != nullptr);
  QSpinBox* recoverySpin =
      dialog.findChild<QSpinBox*>("memoryRecoveryThresholdSpin");
  QVERIFY(recoverySpin != nullptr);

  QVERIFY(automationCheck->isChecked());
  QCOMPARE(freezeSpin->value(), 5);
  QCOMPARE(hibernateSpin->value(), 30);
  QVERIFY(respectCheck->isChecked());
  QVERIFY(pressureCheck->isChecked());
  QCOMPARE(thresholdSpin->value(), 15);
  QCOMPARE(recoverySpin->value(), 20);

  settings->setTheme("system");
}

void TestSettingsDialog::testMemoryPressureThresholds() {
  SettingsManager* settings = SettingsManager::instance();

  SettingsDialog dialog(nullptr);
  QSpinBox* thresholdSpin =
      dialog.findChild<QSpinBox*>("memoryThresholdSpin");
  QVERIFY(thresholdSpin != nullptr);
  QSpinBox* recoverySpin =
      dialog.findChild<QSpinBox*>("memoryRecoveryThresholdSpin");
  QVERIFY(recoverySpin != nullptr);

  // Recovery threshold must be >= activation threshold + 1
  thresholdSpin->setValue(15);
  recoverySpin->setValue(20);
  QMetaObject::invokeMethod(&dialog, "apply", Qt::DirectConnection);

  QCOMPARE(settings->memoryPressureThresholdPercent(), 15);
  QCOMPARE(settings->memoryPressureRecoveryThresholdPercent(), 20);

  settings->setTheme("system");
  settings->setMemoryPressureThresholdPercent(15);
  settings->setMemoryPressureRecoveryThresholdPercent(20);
}

void TestSettingsDialog::testContrastLightTheme() {
  Theme::applyTheme("light");
  QApplication::processEvents();

  SettingsDialog dialog(nullptr);
  dialog.show();
  QApplication::processEvents();

  // Check that all controls have readable colors
  QComboBox* themeCombo = dialog.findChild<QComboBox*>("themeCombo");
  QVERIFY(themeCombo != nullptr);
  QSpinBox* freezeSpin = dialog.findChild<QSpinBox*>("freezeSpin");
  QVERIFY(freezeSpin != nullptr);
  QCheckBox* automationCheck = dialog.findChild<QCheckBox*>("automationCheck");
  QVERIFY(automationCheck != nullptr);

  // Verify palette has correct contrast colors
  const QPalette comboPalette = themeCombo->palette();
  QVERIFY(comboPalette.color(QPalette::Base) == QColor("#ffffff"));
  QVERIFY(comboPalette.color(QPalette::Text) == QColor("#212529"));
  QVERIFY(comboPalette.color(QPalette::PlaceholderText) == QColor("#868e96"));

  const QPalette spinPalette = freezeSpin->palette();
  QVERIFY(spinPalette.color(QPalette::Base) == QColor("#ffffff"));
  QVERIFY(spinPalette.color(QPalette::Text) == QColor("#212529"));

  const QPalette checkPalette = automationCheck->palette();
  // Checkbox should have readable text
  QVERIFY(checkPalette.color(QPalette::WindowText) == QColor("#212529") ||
          checkPalette.color(QPalette::Text) == QColor("#212529"));
}

void TestSettingsDialog::testContrastDarkTheme() {
  Theme::applyTheme("dark");
  QApplication::processEvents();

  SettingsDialog dialog(nullptr);
  dialog.show();
  QApplication::processEvents();

  QComboBox* themeCombo = dialog.findChild<QComboBox*>("themeCombo");
  QVERIFY(themeCombo != nullptr);
  QSpinBox* freezeSpin = dialog.findChild<QSpinBox*>("freezeSpin");
  QVERIFY(freezeSpin != nullptr);
  QCheckBox* automationCheck = dialog.findChild<QCheckBox*>("automationCheck");
  QVERIFY(automationCheck != nullptr);

  const QPalette comboPalette = themeCombo->palette();
  QVERIFY(comboPalette.color(QPalette::Base) == QColor("#242424"));
  QVERIFY(comboPalette.color(QPalette::Text) == QColor("#e0e0e0"));
  QVERIFY(comboPalette.color(QPalette::PlaceholderText) == QColor("#9a9a9a"));

  const QPalette spinPalette = freezeSpin->palette();
  QVERIFY(spinPalette.color(QPalette::Base) == QColor("#242424"));
  QVERIFY(spinPalette.color(QPalette::Text) == QColor("#e0e0e0"));

  const QPalette checkPalette = automationCheck->palette();
  QVERIFY(checkPalette.color(QPalette::WindowText) == QColor("#e0e0e0") ||
          checkPalette.color(QPalette::Text) == QColor("#e0e0e0"));
}

void TestSettingsDialog::testFormLabelsVisible() {
  Theme::applyTheme("light");
  QApplication::processEvents();

  SettingsDialog dialog(nullptr);
  dialog.show();
  QApplication::processEvents();

  // Find all QLabel children in the dialog
  const QList<QLabel*> labels = dialog.findChildren<QLabel*>();
  QVERIFY(!labels.isEmpty());

  // At least one label should contain "Appearance" or "Lifecycle"
  bool foundAppearance = false;
  bool foundLifecycle = false;
  for (QLabel* label : labels) {
    if (label->text().contains("Appearance", Qt::CaseInsensitive)) {
      foundAppearance = true;
    }
    if (label->text().contains("Lifecycle", Qt::CaseInsensitive)) {
      foundLifecycle = true;
    }
  }
  QVERIFY(foundAppearance);
  QVERIFY(foundLifecycle);

  Theme::applyTheme("dark");
  QApplication::processEvents();

  SettingsDialog dialog2(nullptr);
  dialog2.show();
  QApplication::processEvents();

  const QList<QLabel*> labels2 = dialog2.findChildren<QLabel*>();
  QVERIFY(!labels2.isEmpty());

  bool foundAppearance2 = false;
  bool foundLifecycle2 = false;
  for (QLabel* label : labels2) {
    if (label->text().contains("Appearance", Qt::CaseInsensitive)) {
      foundAppearance2 = true;
    }
    if (label->text().contains("Lifecycle", Qt::CaseInsensitive)) {
      foundLifecycle2 = true;
    }
  }
  QVERIFY(foundAppearance2);
  QVERIFY(foundLifecycle2);
}

void TestSettingsDialog::testPlaceholderTextContrast() {
  Theme::applyTheme("light");
  QApplication::processEvents();

  SettingsDialog dialog(nullptr);
  dialog.show();
  QApplication::processEvents();

  QLineEdit* searchEdit = dialog.findChild<QLineEdit*>();
  // SettingsDialog doesn't have QLineEdit, but let's check combo box placeholder
  QComboBox* themeCombo = dialog.findChild<QComboBox*>("themeCombo");
  QVERIFY(themeCombo != nullptr);

  const QPalette palette = themeCombo->palette();
  const QColor placeholderColor = palette.color(QPalette::PlaceholderText);
  QCOMPARE(placeholderColor, QColor("#868e96"));

  Theme::applyTheme("dark");
  QApplication::processEvents();

  SettingsDialog dialog2(nullptr);
  dialog2.show();
  QApplication::processEvents();

  QComboBox* themeCombo2 = dialog2.findChild<QComboBox*>("themeCombo");
  QVERIFY(themeCombo2 != nullptr);

  const QPalette palette2 = themeCombo2->palette();
  const QColor placeholderColor2 = palette2.color(QPalette::PlaceholderText);
  QCOMPARE(placeholderColor2, QColor("#9a9a9a"));
}

void TestSettingsDialog::testDisabledStateContrast() {
  Theme::applyTheme("light");
  QApplication::processEvents();

  SettingsDialog dialog(nullptr);
  dialog.show();
  QApplication::processEvents();

  QSpinBox* freezeSpin = dialog.findChild<QSpinBox*>("freezeSpin");
  QVERIFY(freezeSpin != nullptr);
  freezeSpin->setEnabled(false);
  QApplication::processEvents();

  // Check application palette for disabled state colors
  const QPalette appPalette = qApp->palette();
  const QColor disabledText = appPalette.color(QPalette::Disabled, QPalette::Text);
  const QColor disabledBase = appPalette.color(QPalette::Disabled, QPalette::Base);
  // Disabled text should be readable (not invisible)
  QVERIFY(disabledText == QColor("#adb5bd"));
  // Base might be different, just verify it's not the same as active text (readable)
  QVERIFY(disabledBase != QColor("#212529"));

  Theme::applyTheme("dark");
  QApplication::processEvents();

  SettingsDialog dialog2(nullptr);
  dialog2.show();
  QApplication::processEvents();

  QSpinBox* freezeSpin2 = dialog2.findChild<QSpinBox*>("freezeSpin");
  QVERIFY(freezeSpin2 != nullptr);
  freezeSpin2->setEnabled(false);
  QApplication::processEvents();

  const QPalette appPalette2 = qApp->palette();
  const QColor disabledText2 = appPalette2.color(QPalette::Disabled, QPalette::Text);
  const QColor disabledBase2 = appPalette2.color(QPalette::Disabled, QPalette::Base);
  QVERIFY(disabledText2 == QColor("#5e5c64"));
  QVERIFY(disabledBase2 != QColor("#e0e0e0"));
}

void TestSettingsDialog::testFocusIndicatorsVisible() {
  Theme::applyTheme("light");
  QApplication::processEvents();

  SettingsDialog dialog(nullptr);
  dialog.show();
  QApplication::processEvents();

  QComboBox* themeCombo = dialog.findChild<QComboBox*>("themeCombo");
  QVERIFY(themeCombo != nullptr);

  // Focus style should be defined in stylesheet
  const QString sheet = Theme::styleSheetForMode(Theme::Mode::Light);
  QVERIFY(sheet.contains("QLineEdit:focus") || sheet.contains("QComboBox:focus"));
  QVERIFY(sheet.contains("border: 2px solid #3584e4"));

  Theme::applyTheme("dark");
  QApplication::processEvents();

  SettingsDialog dialog2(nullptr);
  dialog2.show();
  QApplication::processEvents();

  const QString sheet2 = Theme::styleSheetForMode(Theme::Mode::Dark);
  QVERIFY(sheet2.contains("QLineEdit:focus") || sheet2.contains("QComboBox:focus"));
  QVERIFY(sheet2.contains("border: 2px solid #78aeed"));
}

QTEST_MAIN(TestSettingsDialog)
#include "test_settingsdialog.moc"
