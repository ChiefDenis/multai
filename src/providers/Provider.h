#pragma once

#include <QObject>
#include <QString>
#include <QStringList>
#include <QUrl>
#include <QIcon>

class Profile;

class Provider : public QObject {
  Q_OBJECT
public:
  explicit Provider(const QString& id, const QString& displayName, const QUrl& url, QObject* parent = nullptr);

  QString id() const { return m_id; }
  QString displayName() const { return m_displayName; }
  QUrl url() const { return m_url; }
  QIcon icon() const { return m_icon; }

  void setIcon(const QIcon& icon) { m_icon = icon; }
  void setAllowedDomains(const QStringList& domains) { m_allowedDomains = domains; }
  QStringList allowedDomains() const { return m_allowedDomains; }
  bool isUrlAllowed(const QUrl& url) const;

private:
  QString m_id;
  QString m_displayName;
  QUrl m_url;
  QIcon m_icon;
  QStringList m_allowedDomains;
};
