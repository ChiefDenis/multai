#pragma once

#include <QString>
#include <QUrl>

enum class LifecycleState {
  Unloaded,
  Loading,
  Active,
  Frozen,
  Discarded,
  Closing
};

inline QString lifecycleStateToString(LifecycleState state) {
  switch (state) {
    case LifecycleState::Unloaded: return "Unloaded";
    case LifecycleState::Loading: return "Loading";
    case LifecycleState::Active: return "Active";
    case LifecycleState::Frozen: return "Frozen";
    case LifecycleState::Discarded: return "Discarded";
    case LifecycleState::Closing: return "Closing";
  }
  return "Unknown";
}

inline LifecycleState lifecycleStateFromString(const QString& value) {
  if (value == "Loading") return LifecycleState::Loading;
  if (value == "Active") return LifecycleState::Active;
  if (value == "Frozen") return LifecycleState::Frozen;
  if (value == "Discarded") return LifecycleState::Discarded;
  if (value == "Closing") return LifecycleState::Closing;
  return LifecycleState::Unloaded;
}

inline bool canTransition(LifecycleState from, LifecycleState to) {
  if (from == to) {
    return false;
  }

  switch (from) {
    case LifecycleState::Unloaded:
      return to == LifecycleState::Loading || to == LifecycleState::Closing;
    case LifecycleState::Loading:
      return to == LifecycleState::Active || to == LifecycleState::Unloaded || to == LifecycleState::Closing;
    case LifecycleState::Active:
      return to == LifecycleState::Frozen || to == LifecycleState::Discarded || to == LifecycleState::Loading ||
             to == LifecycleState::Closing;
    case LifecycleState::Frozen:
      return to == LifecycleState::Active || to == LifecycleState::Discarded || to == LifecycleState::Closing;
    case LifecycleState::Discarded:
      return to == LifecycleState::Loading || to == LifecycleState::Unloaded || to == LifecycleState::Closing;
    case LifecycleState::Closing:
      return false;
  }
  return false;
}

class TabMetadata {
public:
  TabMetadata();
  TabMetadata(const QString& providerId, const QString& profileId, const QUrl& url);

  QString providerId() const { return m_providerId; }
  QString profileId() const { return m_profileId; }
  QUrl url() const { return m_url; }
  void setUrl(const QUrl& url) { m_url = url; }
  LifecycleState lifecycleState() const { return m_lifecycleState; }
  void setLifecycleState(LifecycleState state) { m_lifecycleState = state; }

  QString title() const { return m_title; }
  void setTitle(const QString& title) { m_title = title; }

  qint64 lastActiveTime() const { return m_lastActiveTime; }
  void setLastActiveTime(qint64 time) { m_lastActiveTime = time; }

private:
  QString m_providerId;
  QString m_profileId;
  QUrl m_url;
  LifecycleState m_lifecycleState;
  QString m_title;
  qint64 m_lastActiveTime;
};
