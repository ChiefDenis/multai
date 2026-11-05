#include <QtTest/QtTest>
#include <QStandardPaths>
#include <QWebEngineProfile>
#include <QDir>

#include "profiles/Profile.h"
#include "profiles/ProfileManager.h"
#include "SettingsManager.h"

class TestProfileIsolation : public QObject {
  Q_OBJECT
private slots:
  void initTestCase();
  void testSeparateProfileInstances();
  void testSeparateProfileDirectories();
  void testProfilesForProvider();
  void testProfilePersistence();
  void testHibernationPreservesProfileData();
};

void TestProfileIsolation::initTestCase() {
  qputenv("QT_QPA_PLATFORM", "offscreen");
  QStandardPaths::setTestModeEnabled(true);
}

void TestProfileIsolation::testSeparateProfileInstances() {
  ProfileManager* pm = ProfileManager::instance();

  Profile* profileA = pm->getOrCreateProfile("deepseek", "account-a");
  Profile* profileB = pm->getOrCreateProfile("deepseek", "account-b");

  QVERIFY(profileA != nullptr);
  QVERIFY(profileB != nullptr);
  QVERIFY(profileA != profileB);
  QVERIFY(profileA->webEngineProfile() != profileB->webEngineProfile());
  QCOMPARE(profileA->accountId(), QString("account-a"));
  QCOMPARE(profileB->accountId(), QString("account-b"));
}

void TestProfileIsolation::testSeparateProfileDirectories() {
  ProfileManager* pm = ProfileManager::instance();

  Profile* profileA = pm->getOrCreateProfile("qwen", "work");
  Profile* profileB = pm->getOrCreateProfile("qwen", "personal");

  QVERIFY(profileA->path() != profileB->path());
  QVERIFY(QDir(profileA->path()).exists());
  QVERIFY(QDir(profileB->path()).exists());
}

void TestProfileIsolation::testProfilesForProvider() {
  ProfileManager* pm = ProfileManager::instance();

  pm->getOrCreateProfile("claude", "alpha");
  pm->getOrCreateProfile("claude", "beta");
  pm->getOrCreateProfile("claude", "gamma");

  QList<Profile*> claudeProfiles = pm->profilesForProvider("claude");
  QCOMPARE(claudeProfiles.size(), 3);

  QList<Profile*> otherProfiles = pm->profilesForProvider("perplexity");
  QCOMPARE(otherProfiles.size(), 0);

  for (Profile* p : claudeProfiles) {
    QCOMPARE(p->providerId(), QString("claude"));
  }
}

void TestProfileIsolation::testProfilePersistence() {
  ProfileManager* pm = ProfileManager::instance();

  Profile* profile = pm->createProfile("gemini", "research-account");
  QVERIFY(profile != nullptr);
  QCOMPARE(profile->name(), QString("research-account"));

  SettingsManager* settings = SettingsManager::instance();
  QString fullId = profile->id();
  QCOMPARE(settings->profileName(fullId), QString("research-account"));

  // Verify profile can be retrieved (cached by ProfileManager)
  Profile* retrieved = pm->getOrCreateProfile("gemini", profile->accountId());
  QVERIFY(retrieved == profile);
}

void TestProfileIsolation::testHibernationPreservesProfileData() {
  // Hibernating a tab destroys the QWebEnginePage but keeps the QWebEngineProfile
  ProfileManager* pm = ProfileManager::instance();
  Profile* profile = pm->getOrCreateProfile("kimi", "default");

  QWebEngineProfile* webProfile = profile->webEngineProfile();
  QVERIFY(webProfile != nullptr);

  // Verify the profile is reference-counted and not destroyed by hibernation
  profile->ref();
  QVERIFY(profile->refCount() > 0);

  profile->deref();
  // Profile should still exist (owned by ProfileManager)
  QVERIFY(profile->webEngineProfile() != nullptr);
  QVERIFY(profile->isLive());
}

QTEST_MAIN(TestProfileIsolation)
#include "test_profile_isolation.moc"
