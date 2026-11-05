#include <QtTest/QtTest>
#include <QCheckBox>
#include <QComboBox>
#include <QSpinBox>
#include <QStandardPaths>
#include "ui/SettingsDialog.h"
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

QTEST_MAIN(TestSettingsDialog)
#include "test_settingsdialog.moc"
