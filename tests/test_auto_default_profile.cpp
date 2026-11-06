#include <QtTest/QtTest>
#include <QSignalSpy>
#include <QStandardPaths>

#include "Sidebar.h"
#include "BrowserWorkspace.h"
#include "lifecycle/TabMetadata.h"
#include "profiles/Profile.h"
#include "profiles/ProfileManager.h"
#include "SettingsManager.h"

class TestAutoDefaultProfile : public QObject {
  Q_OBJECT
private slots:
  void initTestCase();
  void testAutoCreatesDefaultProfileOnFirstUse();
  void testReusesDefaultProfileOnSubsequentUse();
  void testMultipleConversationsShareProfile();
  void testExplicitAdditionalAccountCreation();
};

void TestAutoDefaultProfile::initTestCase() {
  qputenv("QT_QPA_PLATFORM", "offscreen");
  QStandardPaths::setTestModeEnabled(true);
}

void TestAutoDefaultProfile::testAutoCreatesDefaultProfileOnFirstUse() {
  ProfileManager* pm = ProfileManager::instance();

  // Ensure clean state for deepseek
  QList<Profile*> existing = pm->profilesForProvider("deepseek");
  for (Profile* p : existing) {
    pm->deleteProfile(p);
  }

  QCOMPARE(pm->profilesForProvider("deepseek").size(), 0);

  // Simulate first-use: getOrCreateProfile creates "Default" account
  Profile* profile = pm->getOrCreateProfile("deepseek", "Default");
  QVERIFY(profile != nullptr);
  QCOMPARE(profile->accountId(), QString("Default"));
  QCOMPARE(profile->providerId(), QString("deepseek"));

  QCOMPARE(pm->profilesForProvider("deepseek").size(), 1);
}

void TestAutoDefaultProfile::testReusesDefaultProfileOnSubsequentUse() {
  ProfileManager* pm = ProfileManager::instance();

  QList<Profile*> profiles = pm->profilesForProvider("deepseek");
  QVERIFY(!profiles.isEmpty());

  // getOrCreateProfile with same accountId returns the same instance
  Profile* profile1 = pm->getOrCreateProfile("deepseek", "Default");
  Profile* profile2 = pm->getOrCreateProfile("deepseek", "Default");
  QCOMPARE(profile1, profile2);
}

void TestAutoDefaultProfile::testMultipleConversationsShareProfile() {
  ProfileManager* pm = ProfileManager::instance();

  QList<Profile*> profiles = pm->profilesForProvider("deepseek");
  QVERIFY(!profiles.isEmpty());

  Profile* profile = profiles.first();

  // Multiple BrowserTabs can share the same profile - verified via ProfileManager
  // The same profile instance is returned for the same accountId
  Profile* profileAgain = pm->getOrCreateProfile("deepseek", profile->accountId());
  QCOMPARE(profile, profileAgain);
  QCOMPARE(profile->id(), profileAgain->id());
}

void TestAutoDefaultProfile::testExplicitAdditionalAccountCreation() {
  ProfileManager* pm = ProfileManager::instance();

  // First create a default profile for qwen
  Profile* defaultProfile = pm->getOrCreateProfile("qwen", "Default");
  QVERIFY(defaultProfile != nullptr);

  // Then create an additional account
  Profile* profile = pm->createProfile("qwen", "Work Account");
  QVERIFY(profile != nullptr);
  QCOMPARE(profile->name(), QString("Work Account"));
  QCOMPARE(profile->providerId(), QString("qwen"));

  QList<Profile*> qwenProfiles = pm->profilesForProvider("qwen");
  QCOMPARE(qwenProfiles.size(), 2); // default + work account
}

QTEST_MAIN(TestAutoDefaultProfile)
#include "test_auto_default_profile.moc"
