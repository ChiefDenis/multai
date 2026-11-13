#pragma once

#include <QObject>
#include <QString>

class QWebEngineProfile;

class Profile : public QObject {
  Q_OBJECT
public:
  Profile(const QString& providerId, const QString& accountId, const QString& name,
          QObject* parent = nullptr);
  ~Profile() override;

  QString id() const;
  QString storageName() const;
  QString providerId() const { return m_providerId; }
  QString accountId() const { return m_accountId; }
  QString name() const { return m_name; }
  void setName(const QString& name);

  QString path() const;

  QWebEngineProfile* webEngineProfile();
  bool isLive() const { return m_webEngineProfile != nullptr; }

  int refCount() const { return m_refCount; }
  void ref();
  void deref();

signals:
  void nameChanged(const QString& name);

private:
  QString m_providerId;
  QString m_accountId;
  QString m_name;
  QWebEngineProfile* m_webEngineProfile;
  int m_refCount;
};
