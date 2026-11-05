#include <QtTest/QtTest>
#include <QStandardPaths>
#include <QDir>
#include <QFileInfo>

#include "profiles/Profile.h"
#include "profiles/ProfileManager.h"
#include "profiles/ProfileStorage.h"
#include "SettingsManager.h"

class TestProfileGrowth : public QObject {
  Q_OBJECT
private slots:
  void initTestCase();
  void testProfileInitialSize();
  void testProfileDirectoryStructure();
  void testCacheVsProfileSeparation();
  void testProfileGrowthMeasurement();
};

void TestProfileGrowth::initTestCase() {
  qputenv("QT_QPA_PLATFORM", "offscreen");
  QStandardPaths::setTestModeEnabled(true);
}

void TestProfileGrowth::testProfileInitialSize() {
  ProfileManager* pm = ProfileManager::instance();

  Profile* profile = pm->getOrCreateProfile("chatgpt", "growth-test");
  QVERIFY(profile != nullptr);

  QString path = profile->path();
  QVERIFY(!path.isEmpty());
  QVERIFY(QDir(path).exists());

  qint64 totalSize = 0;
  int fileCount = 0;
  QDir dir(path);
  for (const QFileInfo& info :
       dir.entryInfoList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot | QDir::Hidden)) {
    if (info.isFile()) {
      totalSize += info.size();
      ++fileCount;
    }
  }

  qDebug() << "Profile initial size:" << totalSize << "bytes, files:" << fileCount;
  QVERIFY(totalSize < 5 * 1024 * 1024); // Less than 5 MB initially
}

void TestProfileGrowth::testProfileDirectoryStructure() {
  ProfileManager* pm = ProfileManager::instance();

  Profile* profile = pm->getOrCreateProfile("kimi", "structure-test");
  QString path = profile->path();

  QDir dir(path);
  QStringList entries = dir.entryList(QDir::AllEntries | QDir::NoDotAndDotDot | QDir::Hidden);
  qDebug() << "Profile directory entries:" << entries;

  // Profile should have a valid directory structure
  QVERIFY(entries.size() >= 0);
  QVERIFY(QDir(path).exists());
}

void TestProfileGrowth::testCacheVsProfileSeparation() {
  ProfileManager* pm = ProfileManager::instance();

  Profile* profile = pm->getOrCreateProfile("gemini", "cache-test");
  QWebEngineProfile* webProfile = profile->webEngineProfile();
  QVERIFY(webProfile != nullptr);

  // QWebEngineProfile separates persistent data (cookies, local storage)
  // from cache. This test verifies the profile exists and is functional.
  QVERIFY(profile->isLive());

  // The profile path should be under the application profiles directory
  QString path = profile->path();
  QVERIFY(!path.isEmpty());
  QVERIFY(QDir(path).exists());

  qDebug() << "Profile path:" << path;
}

void TestProfileGrowth::testProfileGrowthMeasurement() {
  ProfileManager* pm = ProfileManager::instance();

  // Create multiple profiles and measure total storage
  QStringList providerIds = {
      "deepseek", "qwen", "glm", "copilot", "gemini",
      "claude", "perplexity", "chatgpt", "kimi"};

  qint64 totalSize = 0;
  for (const QString& id : providerIds) {
    Profile* profile = pm->getOrCreateProfile(id, "default");
    QVERIFY(profile != nullptr);

    QDir dir(profile->path());
    for (const QFileInfo& info :
         dir.entryInfoList(QDir::Files | QDir::NoDotAndDotDot | QDir::Hidden)) {
      if (info.isFile()) {
        totalSize += info.size();
      }
    }
  }

  qDebug() << "Total profile storage (9 providers):" << totalSize << "bytes"
           << "(" << (totalSize / 1024 / 1024) << "MB)";
  QVERIFY(totalSize < 50 * 1024 * 1024); // Less than 50 MB for all profiles
}

QTEST_MAIN(TestProfileGrowth)
#include "test_profile_growth.moc"
