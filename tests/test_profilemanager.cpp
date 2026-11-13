#include <QtTest/QtTest>
#include <QStandardPaths>
#include "Profile.h"
#include "SettingsManager.h"
#include "profiles/ProfileManager.h"
#include "profiles/ProfileStorage.h"

class TestProfileManager : public QObject {
  Q_OBJECT
private slots:
  void initTestCase();
  void cleanup();
  void testAccountIdFromName();
  void testGetOrCreateProfile();
  void testGetOrCreateIsIdempotent();
  void testCreateProfileLifecycle();
  void testRenameProfilePersists();
  void testProfilesForProvider();
  void testDeleteProfileRemovesData();
  void testDeleteProfileRefusedWhileInUse();
  void testLoadProfilesScansStorage();
  void testResolveProfileReturnsSavedAccount();
  void testResolveProfileFallsBackToExistingAccount();
  void testResolveProfileCreatesDefaultAccount();
};

void TestProfileManager::initTestCase() {
  QStandardPaths::setTestModeEnabled(true);
}

void TestProfileManager::cleanup() {
  ProfileManager* manager = ProfileManager::instance();
  const QList<Profile*> profiles = manager->profiles();
  for (Profile* profile : profiles) {
    while (profile->refCount() > 0) {
      profile->deref();
    }
    manager->deleteProfile(profile);
  }
}

void TestProfileManager::testAccountIdFromName() {
  QCOMPARE(ProfileManager::accountIdFromName("My Work Account"), QString("my-work-account"));
  QCOMPARE(ProfileManager::accountIdFromName("Personal"), QString("personal"));
  QCOMPARE(ProfileManager::accountIdFromName(""), QString("account"));
  QCOMPARE(ProfileManager::accountIdFromName("   "), QString("account"));
  QCOMPARE(ProfileManager::accountIdFromName("A!!B??C"), QString("a-b-c"));
}

void TestProfileManager::testGetOrCreateProfile() {
  ProfileManager* manager = ProfileManager::instance();

  Profile* profile = manager->getOrCreateProfile("deepseek", "default");
  QVERIFY(profile != nullptr);
  QCOMPARE(profile->id(), QString("deepseek:default"));
  QCOMPARE(profile->accountId(), QString("default"));
  QCOMPARE(profile->providerId(), QString("deepseek"));
  QVERIFY(ProfileStorage::instance()->profileExists("deepseek:default"));
  QVERIFY(!profile->isLive());
  QCOMPARE(profile->refCount(), 0);
}

void TestProfileManager::testGetOrCreateIsIdempotent() {
  ProfileManager* manager = ProfileManager::instance();
  Profile* first = manager->getOrCreateProfile("qwen", "default");
  Profile* second = manager->getOrCreateProfile("qwen", "default");
  QVERIFY(first == second);
  QCOMPARE(manager->profilesForProvider("qwen").size(), 1);
}

void TestProfileManager::testCreateProfileLifecycle() {
  ProfileManager* manager = ProfileManager::instance();

  Profile* personal = manager->createProfile("chatgpt", "Personal");
  QVERIFY(personal != nullptr);
  QCOMPARE(personal->accountId(), QString("personal"));
  QCOMPARE(personal->name(), QString("Personal"));

  Profile* duplicate = manager->createProfile("chatgpt", "Personal");
  QVERIFY(duplicate != nullptr);
  QVERIFY(duplicate != personal);
  QCOMPARE(duplicate->accountId(), QString("personal-2"));

  QCOMPARE(manager->profilesForProvider("chatgpt").size(), 2);
}

void TestProfileManager::testRenameProfilePersists() {
  ProfileManager* manager = ProfileManager::instance();
  Profile* profile = manager->getOrCreateProfile("claude", "default");

  QVERIFY(manager->renameProfile(profile, "Primary Account"));
  QCOMPARE(profile->name(), QString("Primary Account"));
  QCOMPARE(SettingsManager::instance()->profileName("claude:default"),
           QString("Primary Account"));

  QVERIFY(!manager->renameProfile(profile, ""));
  QVERIFY(!manager->renameProfile(nullptr, "nope"));
}

void TestProfileManager::testProfilesForProvider() {
  ProfileManager* manager = ProfileManager::instance();
  manager->getOrCreateProfile("gemini", "default");

  QList<Profile*> profiles = manager->profilesForProvider("gemini");
  QCOMPARE(profiles.size(), 1);
  QVERIFY(manager->profilesForProvider("unknown-provider").isEmpty());
  QVERIFY(manager->profileById("gemini:default") == profiles.at(0));
  QVERIFY(manager->profileById("gemini:missing") == nullptr);
}

void TestProfileManager::testDeleteProfileRemovesData() {
  ProfileManager* manager = ProfileManager::instance();
  Profile* profile = manager->getOrCreateProfile("kimi", "temp");
  QVERIFY(ProfileStorage::instance()->profileExists("kimi:temp"));

  QVERIFY(manager->deleteProfile(profile));
  QVERIFY(!ProfileStorage::instance()->profileExists("kimi:temp"));
  QVERIFY(manager->profileById("kimi:temp") == nullptr);
  QCOMPARE(SettingsManager::instance()->profileName("kimi:temp"), QString());
}

void TestProfileManager::testDeleteProfileRefusedWhileInUse() {
  ProfileManager* manager = ProfileManager::instance();
  Profile* profile = manager->getOrCreateProfile("perplexity", "busy");

  profile->ref();
  QVERIFY(!manager->deleteProfile(profile));
  QVERIFY(manager->profileById("perplexity:busy") != nullptr);

  profile->deref();
  QCOMPARE(profile->refCount(), 0);
  QVERIFY(manager->deleteProfile(profile));
}

void TestProfileManager::testLoadProfilesScansStorage() {
  ProfileManager* manager = ProfileManager::instance();
  manager->getOrCreateProfile("glm", "default");

  manager->loadProfiles();

  const QList<Profile*> profiles = manager->profilesForProvider("glm");
  QCOMPARE(profiles.size(), 1);
  QCOMPARE(profiles.at(0)->accountId(), QString("default"));
}

void TestProfileManager::testResolveProfileReturnsSavedAccount() {
  ProfileManager* manager = ProfileManager::instance();
  Profile* work = manager->getOrCreateProfile("deepseek", "work");
  QVERIFY(work != nullptr);

  Profile* resolved = manager->resolveProfile("deepseek", "work");
  QCOMPARE(resolved, work);
  QCOMPARE(resolved->id(), QString("deepseek:work"));
}

void TestProfileManager::testResolveProfileFallsBackToExistingAccount() {
  ProfileManager* manager = ProfileManager::instance();
  Profile* work = manager->getOrCreateProfile("deepseek", "work");
  QVERIFY(work != nullptr);
  QVERIFY(manager->resolveProfile("deepseek", "deleted-account") != nullptr);
  QCOMPARE(manager->resolveProfile("deepseek", "deleted-account"), work);
  QCOMPARE(manager->profilesForProvider("deepseek").size(), 1);
}

void TestProfileManager::testResolveProfileCreatesDefaultAccount() {
  ProfileManager* manager = ProfileManager::instance();
  Profile* resolved = manager->resolveProfile("kimi", "deleted-account");
  QVERIFY(resolved != nullptr);
  QCOMPARE(resolved->accountId(), QString("default"));
  QVERIFY(ProfileStorage::instance()->profileExists("kimi:default"));
}

QTEST_MAIN(TestProfileManager)
#include "test_profilemanager.moc"
