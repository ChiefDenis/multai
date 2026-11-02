#include "TabMetadata.h"

TabMetadata::TabMetadata()
  : m_lifecycleState(LifecycleState::Unloaded)
  , m_lastActiveTime(0) {
}

TabMetadata::TabMetadata(const QString& providerId, const QString& profileId, const QUrl& url)
  : m_providerId(providerId)
  , m_profileId(profileId)
  , m_url(url)
  , m_lifecycleState(LifecycleState::Unloaded)
  , m_lastActiveTime(0) {
}
