#include <QtTest/QtTest>
#include <QStandardPaths>

#include "SettingsManager.h"

class TestSettingsManager : public QObject {
  Q_OBJECT
private slots:
  void initTestCase();
  void testInstance();
  void testValue();
  void testSetValue();
  void testReset();
  void testTheme();
  void testWindowGeometry();
};

void TestSettingsManager::initTestCase() {
  QStandardPaths::setTestModeEnabled(true);
}

void TestSettingsManager::testInstance() {
  SettingsManager* instance1 = SettingsManager::instance();
  SettingsManager* instance2 = SettingsManager::instance();
  QCOMPARE(instance1, instance2);
}

void TestSettingsManager::testValue() {
  SettingsManager* settings = SettingsManager::instance();

  QVariant value = settings->value("test_key", "default");
  QCOMPARE(value.toString(), QString("default"));

  settings->setValue("test_key", "custom");
  value = settings->value("test_key", "default");
  QCOMPARE(value.toString(), QString("custom"));

  settings->reset("test_key");
}

void TestSettingsManager::testSetValue() {
  SettingsManager* settings = SettingsManager::instance();

  settings->setValue("test_int", 42);
  QCOMPARE(settings->value("test_int").toInt(), 42);

  settings->setValue("test_string", "hello");
  QCOMPARE(settings->value("test_string").toString(), QString("hello"));

  settings->reset("test_int");
  settings->reset("test_string");
}

void TestSettingsManager::testReset() {
  SettingsManager* settings = SettingsManager::instance();

  settings->setValue("test_reset", "value");
  QVERIFY(settings->value("test_reset").isValid());

  settings->reset("test_reset");
  QVERIFY(!settings->value("test_reset").isValid());
}

void TestSettingsManager::testTheme() {
  SettingsManager* settings = SettingsManager::instance();

  QString theme = settings->theme();
  QVERIFY(theme == "system" || theme == "light" || theme == "dark");

  settings->setTheme("dark");
  QCOMPARE(settings->theme(), QString("dark"));

  settings->setTheme("light");
  QCOMPARE(settings->theme(), QString("light"));

  settings->setTheme("system");
  QCOMPARE(settings->theme(), QString("system"));
}

void TestSettingsManager::testWindowGeometry() {
  SettingsManager* settings = SettingsManager::instance();

  settings->reset("window/geometry");
  QByteArray geometry = settings->windowGeometry();
  QVERIFY(geometry.isEmpty());

  QByteArray testGeometry("test_geometry");
  settings->setWindowGeometry(testGeometry);
  QCOMPARE(settings->windowGeometry(), testGeometry);

  settings->reset("window/geometry");
}

QTEST_MAIN(TestSettingsManager)
#include "test_settingsmanager.moc"
