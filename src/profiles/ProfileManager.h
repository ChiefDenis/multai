#pragma once

#include <QList>
#include <QMap>
#include <QObject>
#include <QString>

class Profile;

class ProfileManager : public QObject {
  Q_OBJECT
public:
  static ProfileManager* instance();

  void loadProfiles();

  Profile* getOrCreateProfile(const QString& providerId, const QString& accountId);
  Profile* resolveProfile(const QString& providerId, const QString& accountId);
  Profile* createProfile(const QString& providerId, const QString& name);
  Profile* profileById(const QString& fullId) const;
  QList<Profile*> profiles() const;
  QList<Profile*> profilesForProvider(const QString& providerId) const;

  bool renameProfile(Profile* profile, const QString& name);
  bool deleteProfile(Profile* profile);

  static QString accountIdFromName(const QString& name);

signals:
  void profileCreated(Profile* profile);
  void profileDeleted(Profile* profile);
  void profilesChanged();

private:
  explicit ProfileManager(QObject* parent = nullptr);
  ~ProfileManager() override;

  QString uniqueAccountId(const QString& providerId, const QString& base) const;

  QMap<QString, Profile*> m_profiles;
};
