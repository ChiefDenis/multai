#include "Provider.h"
#include "ProviderIcons.h"

#include <QRegularExpression>

Provider::Provider(const QString& id, const QString& displayName, const QUrl& url, QObject* parent)
  : QObject(parent)
  , m_id(id)
  , m_displayName(displayName)
  , m_url(url) {
  if (!url.host().isEmpty()) {
    m_allowedDomains.append(url.host());
  }
}

QIcon Provider::icon() const {
  return providerIcon(m_id);
}

bool Provider::isUrlAllowed(const QUrl& url) const {
  if (!url.isValid() || url.isEmpty() || !url.userInfo().isEmpty()) {
    return false;
  }
  if (url.scheme() != "http" && url.scheme() != "https") {
    return false;
  }

  const QString host = url.host().toLower();
  if (host.isEmpty()) {
    return false;
  }

  for (const QString& domain : m_allowedDomains) {
    const QString normalized = domain.toLower().remove(QRegularExpression("^\\.+"));
    if (normalized.isEmpty()) {
      continue;
    }
    if (host == normalized || host.endsWith("." + normalized)) {
      return true;
    }
  }
  return false;
}
