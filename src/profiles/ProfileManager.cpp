#include "ProfileManager.h"
#include "Profile.h"
#include "ProfileStorage.h"
#include "SettingsManager.h"

#include <QRegularExpression>

ProfileManager::ProfileManager(QObject* parent)
  : QObject(parent) {
}

ProfileManager::~ProfileManager() {
  qDeleteAll(m_profiles);
}

ProfileManager* ProfileManager::instance() {
  static ProfileManager instance;
  return &instance;
}

void ProfileManager::loadProfiles() {
  ProfileStorage* storage = ProfileStorage::instance();
  SettingsManager* settings = SettingsManager::instance();

  const QStringList ids = storage->listProfileIds();
  for (const QString& fullId : ids) {
    const int separator = fullId.indexOf(':');
    if (separator <= 0 || m_profiles.contains(fullId)) {
      continue;
    }

    const QString providerId = fullId.left(separator);
    const QString accountId = fullId.mid(separator + 1);

    QString name = settings->profileName(fullId);
    if (name.isEmpty()) {
      name = accountId;
    }

    m_profiles[fullId] = new Profile(providerId, accountId, name, this);
  }

  emit profilesChanged();
}

Profile* ProfileManager::getOrCreateProfile(const QString& providerId, const QString& accountId) {
  const QString fullId = providerId + ":" + accountId;

  if (m_profiles.contains(fullId)) {
    return m_profiles.value(fullId);
  }

  ProfileStorage* storage = ProfileStorage::instance();
  if (!storage->profileExists(fullId) && !storage->createProfileDirectory(fullId)) {
    return nullptr;
  }

  SettingsManager* settings = SettingsManager::instance();
  QString name = settings->profileName(fullId);
  if (name.isEmpty()) {
    name = accountId;
    settings->setProfileName(fullId, name);
  }

  Profile* profile = new Profile(providerId, accountId, name, this);
  m_profiles[fullId] = profile;

  emit profileCreated(profile);
  emit profilesChanged();
  return profile;
}

Profile* ProfileManager::resolveProfile(const QString& providerId, const QString& accountId) {
  if (!accountId.isEmpty()) {
    Profile* saved = profileById(providerId + ":" + accountId);
    if (saved) {
      return saved;
    }
  }

  const QList<Profile*> existing = profilesForProvider(providerId);
  if (!existing.isEmpty()) {
    return existing.first();
  }

  return createProfile(providerId, "Default");
}

Profile* ProfileManager::createProfile(const QString& providerId, const QString& name) {
  const QString accountId = uniqueAccountId(providerId, accountIdFromName(name));
  Profile* profile = getOrCreateProfile(providerId, accountId);
  if (profile && profile->name() != name) {
    renameProfile(profile, name);
  }
  return profile;
}

Profile* ProfileManager::profileById(const QString& fullId) const {
  return m_profiles.value(fullId, nullptr);
}

QList<Profile*> ProfileManager::profiles() const {
  return m_profiles.values();
}

QList<Profile*> ProfileManager::profilesForProvider(const QString& providerId) const {
  QList<Profile*> result;
  for (auto it = m_profiles.constBegin(); it != m_profiles.constEnd(); ++it) {
    if (it.value()->providerId() == providerId) {
      result.append(it.value());
    }
  }
  return result;
}

bool ProfileManager::renameProfile(Profile* profile, const QString& name) {
  if (!profile || name.isEmpty()) {
    return false;
  }

  profile->setName(name);
  SettingsManager::instance()->setProfileName(profile->id(), name);
  emit profilesChanged();
  return true;
}

bool ProfileManager::deleteProfile(Profile* profile) {
  if (!profile || profile->refCount() > 0) {
    return false;
  }

  const QString fullId = profile->id();

  m_profiles.remove(fullId);
  ProfileStorage::instance()->deleteProfileDirectory(fullId);
  SettingsManager::instance()->removeProfileName(fullId);

  emit profileDeleted(profile);
  emit profilesChanged();

  delete profile;
  return true;
}

QString ProfileManager::accountIdFromName(const QString& name) {
  QString slug = name.toLower().simplified();
  slug.replace(QRegularExpression("[^a-z0-9]+"), "-");
  slug.remove(QRegularExpression("^-+|-+$"));
  return slug.isEmpty() ? QString("account") : slug;
}

QString ProfileManager::uniqueAccountId(const QString& providerId, const QString& base) const {
  QString candidate = base;
  int suffix = 2;
  while (m_profiles.contains(providerId + ":" + candidate)) {
    candidate = base + "-" + QString::number(suffix++);
  }
  return candidate;
}
