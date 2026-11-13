#pragma once

#include <QObject>
#include <QString>
#include <QStringList>

class ProfileStorage : public QObject {
  Q_OBJECT
public:
  static ProfileStorage* instance();

  QString basePath() const;
  QString profilePath(const QString& fullId) const;
  QStringList listProfileIds() const;

  bool createProfileDirectory(const QString& fullId);
  bool deleteProfileDirectory(const QString& fullId);
  bool profileExists(const QString& fullId) const;

  bool setPermissions(const QString& path);

private:
  explicit ProfileStorage(QObject* parent = nullptr);
  ~ProfileStorage() override = default;
};
