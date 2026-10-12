#include "Provider.h"

Provider::Provider(const QString& id, const QString& displayName, const QUrl& url, QObject* parent)
  : QObject(parent)
  , m_id(id)
  , m_displayName(displayName)
  , m_url(url) {
}
