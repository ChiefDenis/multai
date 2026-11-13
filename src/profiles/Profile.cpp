#include "Profile.h"
#include "ProfileStorage.h"

#include <QWebEngineProfile>

Profile::Profile(const QString& providerId, const QString& accountId, const QString& name,
                 QObject* parent)
  : QObject(parent)
  , m_providerId(providerId)
  , m_accountId(accountId)
  , m_name(name.isEmpty() ? accountId : name)
  , m_webEngineProfile(nullptr)
  , m_refCount(0) {
}

Profile::~Profile() {
  delete m_webEngineProfile;
  m_webEngineProfile = nullptr;
}

QString Profile::id() const {
  return m_providerId + ":" + m_accountId;
}

QString Profile::storageName() const {
  return m_providerId + "-" + m_accountId;
}

void Profile::setName(const QString& name) {
  if (name.isEmpty() || name == m_name) {
    return;
  }
  m_name = name;
  emit nameChanged(m_name);
}

QString Profile::path() const {
  return ProfileStorage::instance()->profilePath(id());
}

QWebEngineProfile* Profile::webEngineProfile() {
  if (!m_webEngineProfile) {
    const QString profilePath = path();

    m_webEngineProfile = new QWebEngineProfile(storageName(), nullptr);
    m_webEngineProfile->setPersistentCookiesPolicy(QWebEngineProfile::ForcePersistentCookies);
    m_webEngineProfile->setPersistentStoragePath(profilePath + "/storage");
    m_webEngineProfile->setCachePath(profilePath + "/cache");

    ProfileStorage::instance()->createProfileDirectory(id());
  }
  return m_webEngineProfile;
}

void Profile::ref() {
  ++m_refCount;
}

void Profile::deref() {
  if (m_refCount > 0) {
    --m_refCount;
  }
}
