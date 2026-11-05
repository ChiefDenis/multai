#include <QtTest/QtTest>
#include "MemoryMonitor.h"

class TestMemoryMonitor : public QObject {
  Q_OBJECT
private slots:
  void testParseMemInfo();
  void testParseMemInfoFallback();
  void testParseMemInfoInvalid();
  void testParsePressure();
  void testParsePressureMissing();
  void testReadMemoryInfo();
  void testThresholdClamping();
  void testRecoveryThresholdClamping();
  void testEntersPressureState();
  void testNoRepeatedPressureEvents();
  void testRecoversFromPressure();
  void testReEntersPressureAfterRecovery();
  void testValuesAroundThresholds();
};

void TestMemoryMonitor::testParseMemInfo() {
  const QString content =
      "MemTotal:       16000000 kB\n"
      "MemFree:         1000000 kB\n"
      "MemAvailable:    8000000 kB\n"
      "Buffers:          100000 kB\n"
      "Cached:          2000000 kB\n";

  quint64 total = 0;
  quint64 available = 0;
  QVERIFY(MemoryMonitor::parseMemInfo(content, &total, &available));
  QCOMPARE(total, quint64(16000000) * 1024ULL);
  QCOMPARE(available, quint64(8000000) * 1024ULL);
}

void TestMemoryMonitor::testParseMemInfoFallback() {
  const QString content =
      "MemTotal:       1000000 kB\n"
      "MemFree:         200000 kB\n"
      "Buffers:          50000 kB\n"
      "Cached:          150000 kB\n";

  quint64 total = 0;
  quint64 available = 0;
  QVERIFY(MemoryMonitor::parseMemInfo(content, &total, &available));
  QCOMPARE(available, quint64(400000) * 1024ULL);
}

void TestMemoryMonitor::testParseMemInfoInvalid() {
  quint64 total = 0;
  quint64 available = 0;
  QVERIFY(!MemoryMonitor::parseMemInfo("not meminfo", &total, &available));
  QVERIFY(!MemoryMonitor::parseMemInfo("MemFree: 10 kB", &total, &available));
}

void TestMemoryMonitor::testParsePressure() {
  const QString content =
      "some avg10=1.25 avg60=0.40 avg300=0.10 total=1234\n"
      "full avg10=0.50 avg60=0.20 avg300=0.05 total=567\n";

  double value = 0.0;
  QVERIFY(MemoryMonitor::parsePressure(content, &value));
  QCOMPARE(value, 1.25);
}

void TestMemoryMonitor::testParsePressureMissing() {
  double value = 0.0;
  QVERIFY(!MemoryMonitor::parsePressure("full avg10=0.50 avg60=0.20 avg300=0.05 total=567\n",
                                        &value));
}

void TestMemoryMonitor::testReadMemoryInfo() {
  quint64 total = 0;
  quint64 available = 0;
  QVERIFY(MemoryMonitor::readMemoryInfo(&total, &available));
  QVERIFY(total > 0);
  QVERIFY(available > 0);
  QVERIFY(available <= total);
}

void TestMemoryMonitor::testThresholdClamping() {
  MemoryMonitor monitor;
  monitor.setPressureThresholdPercent(500);
  QCOMPARE(monitor.pressureThresholdPercent(), 90);
  monitor.setPressureThresholdPercent(0);
  QCOMPARE(monitor.pressureThresholdPercent(), 1);
  monitor.setPressureThresholdPercent(25);
  QCOMPARE(monitor.pressureThresholdPercent(), 25);
}

void TestMemoryMonitor::testRecoveryThresholdClamping() {
  MemoryMonitor monitor;
  // Default pressure threshold is 15; recovery must be above it.
  monitor.setRecoveryThresholdPercent(500);
  QCOMPARE(monitor.recoveryThresholdPercent(), 90);
  monitor.setRecoveryThresholdPercent(0);
  QCOMPARE(monitor.recoveryThresholdPercent(), 16);
  monitor.setRecoveryThresholdPercent(25);
  QCOMPARE(monitor.recoveryThresholdPercent(), 25);
}

void TestMemoryMonitor::testEntersPressureState() {
  MemoryMonitor monitor;
  monitor.setPressureThresholdPercent(15);
  monitor.setRecoveryThresholdPercent(20);

  QSignalSpy pressureSpy(&monitor, &MemoryMonitor::memoryPressureDetected);
  QSignalSpy availableSpy(&monitor, &MemoryMonitor::memoryAvailable);

  monitor.setMemoryOverride(10000, 1000); // 10% available
  QMetaObject::invokeMethod(&monitor, "check", Qt::DirectConnection);

  QCOMPARE(pressureSpy.count(), 1);
  QCOMPARE(availableSpy.count(), 0);
  QVERIFY(monitor.isInPressureState());
  QCOMPARE(monitor.availablePercent(), 10);
}

void TestMemoryMonitor::testNoRepeatedPressureEvents() {
  MemoryMonitor monitor;
  monitor.setPressureThresholdPercent(15);
  monitor.setRecoveryThresholdPercent(20);

  QSignalSpy pressureSpy(&monitor, &MemoryMonitor::memoryPressureDetected);

  monitor.setMemoryOverride(10000, 1000); // 10% available
  QMetaObject::invokeMethod(&monitor, "check", Qt::DirectConnection);
  QCOMPARE(pressureSpy.count(), 1);

  QMetaObject::invokeMethod(&monitor, "check", Qt::DirectConnection); // still pressure
  QCOMPARE(pressureSpy.count(), 1);

  monitor.setMemoryOverride(10000, 500); // 5% available, still pressure
  QMetaObject::invokeMethod(&monitor, "check", Qt::DirectConnection);
  QCOMPARE(pressureSpy.count(), 1);
}

void TestMemoryMonitor::testRecoversFromPressure() {
  MemoryMonitor monitor;
  monitor.setPressureThresholdPercent(15);
  monitor.setRecoveryThresholdPercent(20);

  QSignalSpy pressureSpy(&monitor, &MemoryMonitor::memoryPressureDetected);
  QSignalSpy availableSpy(&monitor, &MemoryMonitor::memoryAvailable);

  monitor.setMemoryOverride(10000, 1000); // 10% available -> pressure
  QMetaObject::invokeMethod(&monitor, "check", Qt::DirectConnection);
  QCOMPARE(pressureSpy.count(), 1);
  QVERIFY(monitor.isInPressureState());

  monitor.setMemoryOverride(10000, 2500); // 25% available -> above 20% recovery
  QMetaObject::invokeMethod(&monitor, "check", Qt::DirectConnection);
  QCOMPARE(availableSpy.count(), 1);
  QVERIFY(!monitor.isInPressureState());
}

void TestMemoryMonitor::testReEntersPressureAfterRecovery() {
  MemoryMonitor monitor;
  monitor.setPressureThresholdPercent(15);
  monitor.setRecoveryThresholdPercent(20);

  QSignalSpy pressureSpy(&monitor, &MemoryMonitor::memoryPressureDetected);

  monitor.setMemoryOverride(10000, 1000); // pressure
  QMetaObject::invokeMethod(&monitor, "check", Qt::DirectConnection);
  QCOMPARE(pressureSpy.count(), 1);

  monitor.setMemoryOverride(10000, 2500); // recover
  QMetaObject::invokeMethod(&monitor, "check", Qt::DirectConnection);
  QCOMPARE(pressureSpy.count(), 1);

  monitor.setMemoryOverride(10000, 1000); // re-enter pressure
  QMetaObject::invokeMethod(&monitor, "check", Qt::DirectConnection);
  QCOMPARE(pressureSpy.count(), 2);
}

void TestMemoryMonitor::testValuesAroundThresholds() {
  MemoryMonitor monitor;
  monitor.setPressureThresholdPercent(15);
  monitor.setRecoveryThresholdPercent(20);

  QSignalSpy pressureSpy(&monitor, &MemoryMonitor::memoryPressureDetected);
  QSignalSpy availableSpy(&monitor, &MemoryMonitor::memoryAvailable);

  monitor.setMemoryOverride(10000, 1500); // exactly 15% -> <= threshold, enters pressure
  QMetaObject::invokeMethod(&monitor, "check", Qt::DirectConnection);
  QCOMPARE(pressureSpy.count(), 1);
  QVERIFY(monitor.isInPressureState());

  monitor.setMemoryOverride(10000, 1900); // 19% -> below recovery threshold (20%), still pressure
  QMetaObject::invokeMethod(&monitor, "check", Qt::DirectConnection);
  QCOMPARE(pressureSpy.count(), 1);
  QCOMPARE(availableSpy.count(), 0);
  QVERIFY(monitor.isInPressureState());

  monitor.setMemoryOverride(10000, 2100); // 21% -> above recovery threshold, recovers
  QMetaObject::invokeMethod(&monitor, "check", Qt::DirectConnection);
  QCOMPARE(availableSpy.count(), 1);
  QVERIFY(!monitor.isInPressureState());

  monitor.setMemoryOverride(10000, 1400); // 14% -> below threshold, re-enters pressure
  QMetaObject::invokeMethod(&monitor, "check", Qt::DirectConnection);
  QCOMPARE(pressureSpy.count(), 2);
  QVERIFY(monitor.isInPressureState());
}

QTEST_MAIN(TestMemoryMonitor)
#include "test_memorymonitor.moc"
