#include "ProfileStorage.h"

#include <QDir>
#include <QFile>
#include <QStandardPaths>

ProfileStorage::ProfileStorage(QObject* parent)
  : QObject(parent) {
}

ProfileStorage* ProfileStorage::instance() {
  static ProfileStorage instance;
  return &instance;
}

QString ProfileStorage::basePath() const {
  return QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation) + "/profiles";
}

QString ProfileStorage::profilePath(const QString& fullId) const {
  return basePath() + "/" + fullId;
}

QStringList ProfileStorage::listProfileIds() const {
  QDir dir(basePath());
  if (!dir.exists()) {
    return {};
  }
  return dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);
}

bool ProfileStorage::createProfileDirectory(const QString& fullId) {
  QDir dir;
  if (!dir.mkpath(profilePath(fullId))) {
    return false;
  }
  return setPermissions(profilePath(fullId));
}

bool ProfileStorage::deleteProfileDirectory(const QString& fullId) {
  QDir dir(profilePath(fullId));
  if (!dir.exists()) {
    return true;
  }
  return dir.removeRecursively();
}

bool ProfileStorage::profileExists(const QString& fullId) const {
  return QDir(profilePath(fullId)).exists();
}

bool ProfileStorage::setPermissions(const QString& path) {
  return QFile::setPermissions(path, QFile::ReadOwner | QFile::WriteOwner | QFile::ExeOwner);
}
