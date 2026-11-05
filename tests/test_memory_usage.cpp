#include <QtTest/QtTest>
#include <QStandardPaths>
#include <QFile>
#include <QTextStream>
#include <QRegularExpression>

#include "lifecycle/BrowserTab.h"
#include "lifecycle/LifecycleManager.h"
#include "SettingsManager.h"
#include "profiles/ProfileManager.h"

namespace {

quint64 readVmRSS(qint64 pid) {
  QFile statusFile(QString("/proc/%1/status").arg(pid));
  if (!statusFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
    return 0;
  }
  QTextStream stream(&statusFile);
  while (!stream.atEnd()) {
    QString line = stream.readLine();
    if (line.startsWith("VmRSS:")) {
      QStringList parts = line.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
      if (parts.size() >= 2) {
        return parts[1].toULongLong() * 1024; // kB to bytes
      }
    }
  }
  return 0;
}

}  // namespace

class TestMemoryUsage : public QObject {
  Q_OBJECT
private slots:
  void initTestCase();
  void testBaselineMemoryUsage();
  void testPerTabMemoryActive();
  void testPerTabMemoryFrozen();
  void testPerTabMemoryDiscarded();
  void testProfileStorageSize();
};

void TestMemoryUsage::initTestCase() {
  qputenv("QT_QPA_PLATFORM", "offscreen");
  QStandardPaths::setTestModeEnabled(true);
}

void TestMemoryUsage::testBaselineMemoryUsage() {
  // Measure memory of current test process
  quint64 rss = readVmRSS(QCoreApplication::applicationPid());
  // RSS may be 0 in restricted environments; just record it
  qDebug() << "Test process RSS:" << (rss / 1024 / 1024) << "MB";
  // Don't assert on RSS in headless CI - it depends on environment
}

void TestMemoryUsage::testPerTabMemoryActive() {
  BrowserTab tab("deepseek", "default", QUrl("https://www.deepseek.com/"));

  // Create page
  tab.activate();
  tab.onLoadFinished(true);
  QCOMPARE(tab.lifecycleState(), LifecycleState::Active);

  // Verify page exists
  QVERIFY(tab.page() != nullptr);
  QVERIFY(tab.pageCreated());

  quint64 rss = readVmRSS(QCoreApplication::applicationPid());
  qDebug() << "Active tab RSS:" << (rss / 1024 / 1024) << "MB";

  tab.closeTab();
}

void TestMemoryUsage::testPerTabMemoryFrozen() {
  BrowserTab tab("deepseek", "default", QUrl("https://www.deepseek.com/"));

  tab.activate();
  tab.onLoadFinished(true);
  QCOMPARE(tab.lifecycleState(), LifecycleState::Active);

  SettingsManager::instance()->setRespectRecommendedState(false);
  tab.freeze();
  QCOMPARE(tab.lifecycleState(), LifecycleState::Frozen);

  // Frozen tab should still have a page (not destroyed)
  QVERIFY(tab.page() != nullptr);
  QVERIFY(tab.pageCreated());

  quint64 rss = readVmRSS(QCoreApplication::applicationPid());
  qDebug() << "Frozen tab RSS:" << (rss / 1024 / 1024) << "MB";

  SettingsManager::instance()->setRespectRecommendedState(true);
  tab.closeTab();
}

void TestMemoryUsage::testPerTabMemoryDiscarded() {
  BrowserTab tab("deepseek", "default", QUrl("https://www.deepseek.com/"));

  tab.activate();
  tab.onLoadFinished(true);
  QCOMPARE(tab.lifecycleState(), LifecycleState::Active);

  SettingsManager::instance()->setRespectRecommendedState(false);
  tab.hibernate();
  QCOMPARE(tab.lifecycleState(), LifecycleState::Discarded);

  // Discarded tab should NOT have a page (destroyed to free memory)
  QVERIFY(tab.page() == nullptr);
  QVERIFY(!tab.pageCreated());

  quint64 rss = readVmRSS(QCoreApplication::applicationPid());
  qDebug() << "Discarded tab RSS:" << (rss / 1024 / 1024) << "MB";

  SettingsManager::instance()->setRespectRecommendedState(true);
  tab.closeTab();
}

void TestMemoryUsage::testProfileStorageSize() {
  ProfileManager* pm = ProfileManager::instance();

  // Create a profile
  Profile* profile = pm->getOrCreateProfile("perplexity", "test");
  QVERIFY(profile != nullptr);

  QString path = profile->path();
  QVERIFY(!path.isEmpty());
  QVERIFY(QDir(path).exists());

  qDebug() << "Profile path:" << path;

  // Initial profile size should be minimal
  QDir dir(path);
  qint64 totalSize = 0;
  for (const QFileInfo& info : dir.entryInfoList(QDir::Files | QDir::NoDotAndDotDot | QDir::Hidden)) {
    if (info.isFile()) {
      totalSize += info.size();
    }
  }
  qDebug() << "Initial profile size:" << totalSize << "bytes";
  QVERIFY(totalSize < 10 * 1024 * 1024); // Less than 10 MB initially
}

QTEST_MAIN(TestMemoryUsage)
#include "test_memory_usage.moc"
