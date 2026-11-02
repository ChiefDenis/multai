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

QTEST_MAIN(TestMemoryMonitor)
#include "test_memorymonitor.moc"
