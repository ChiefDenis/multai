#include "DataMigration.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QHash>
#include <QStandardPaths>
#include <QtGlobal>

namespace {

constexpr auto kLegacySettingsFile = "AI Workspace.conf";
constexpr auto kSettingsFile = "Multai.conf";
constexpr auto kLegacyAppDir = "AI Workspace";
constexpr auto kAppDir = "Multai";
constexpr auto kMarkerFile = ".migration-complete";

QString kindName(DataMigration::RootKind kind) {
  switch (kind) {
    case DataMigration::RootKind::Config:
      return QStringLiteral("config");
    case DataMigration::RootKind::Data:
      return QStringLiteral("data");
    case DataMigration::RootKind::Cache:
      return QStringLiteral("cache");
  }
  return {};
}

QString mappedTopLevelName(const QString& name) {
  if (name == QLatin1String(kLegacySettingsFile)) return QString::fromLatin1(kSettingsFile);
  if (name == QLatin1String(kLegacyAppDir)) return QString::fromLatin1(kAppDir);
  return name;
}

bool copyTree(const QString& source, const QString& target, QString* error) {
  const QFileInfo sourceInfo(source);
  if (!QDir().mkpath(target)) {
    *error = QStringLiteral("cannot create %1").arg(target);
    return false;
  }
  QFile::setPermissions(target, sourceInfo.permissions());

  const QFileInfoList entries =
      QDir(source).entryInfoList(QDir::NoDotAndDotDot | QDir::AllEntries | QDir::Hidden |
                                 QDir::System);
  for (const QFileInfo& entry : entries) {
    const QString targetPath = target + QLatin1Char('/') + entry.fileName();
    if (entry.isSymLink()) {
      if (!QFile::link(entry.symLinkTarget(), targetPath)) {
        *error = QStringLiteral("cannot copy symlink %1").arg(entry.absoluteFilePath());
        return false;
      }
    } else if (entry.isDir()) {
      if (!copyTree(entry.absoluteFilePath(), targetPath, error)) return false;
    } else if (entry.isFile()) {
      if (!QFile::copy(entry.absoluteFilePath(), targetPath)) {
        *error = QStringLiteral("cannot copy %1").arg(entry.absoluteFilePath());
        return false;
      }
      QFile::setPermissions(targetPath, entry.permissions());
    }
  }
  return true;
}

void collectStats(const QString& root, const QString& prefix, bool mapTopLevel,
                  QHash<QString, qint64>* stats) {
  const QFileInfoList entries =
      QDir(root).entryInfoList(QDir::NoDotAndDotDot | QDir::AllEntries | QDir::Hidden |
                               QDir::System);
  for (const QFileInfo& entry : entries) {
    QString name = entry.fileName();
    if (prefix.isEmpty() && mapTopLevel) name = mappedTopLevelName(name);

    const QString key = prefix + name;
    if (entry.isDir() && !entry.isSymLink()) {
      stats->insert(key + QLatin1Char('/'), -1);
      collectStats(entry.absoluteFilePath(), key + QLatin1Char('/'), false, stats);
    } else {
      stats->insert(key, entry.size());
    }
  }
}

bool verifyTree(const QString& legacyRoot, const QString& stagingRoot, QString* error) {
  QHash<QString, qint64> expected;
  QHash<QString, qint64> actual;
  collectStats(legacyRoot, {}, true, &expected);
  collectStats(stagingRoot, {}, false, &actual);

  if (expected != actual) {
    *error = QStringLiteral("copied data does not match the legacy tree");
    return false;
  }

  qint64 bytes = 0;
  for (auto it = expected.cbegin(); it != expected.cend(); ++it) {
    if (it.value() >= 0) bytes += it.value();
  }
  qInfo().noquote() << QStringLiteral("Multai migration: verified %1 entries, %2 bytes")
                           .arg(expected.size())
                           .arg(bytes);
  return true;
}

bool renameTopLevel(const QString& stagingRoot, QString* error) {
  const QPair<QString, QString> names[] = {
      {QString::fromLatin1(kLegacySettingsFile), QString::fromLatin1(kSettingsFile)},
      {QString::fromLatin1(kLegacyAppDir), QString::fromLatin1(kAppDir)},
  };

  for (const auto& pair : names) {
    const QString from = stagingRoot + QLatin1Char('/') + pair.first;
    QFileInfo fromInfo(from);
    if (!fromInfo.exists() && !fromInfo.isSymLink()) continue;

    const QString to = stagingRoot + QLatin1Char('/') + pair.second;
    QFileInfo toInfo(to);
    if (toInfo.exists() || toInfo.isSymLink()) {
      *error = QStringLiteral("both legacy and new name exist: %1").arg(to);
      return false;
    }
    if (!QDir().rename(from, to)) {
      *error = QStringLiteral("cannot rename %1 to %2").arg(from, to);
      return false;
    }
  }
  return true;
}

bool writeMarker(const QString& stagingRoot, const QString& legacyRoot) {
  QFile marker(stagingRoot + QLatin1Char('/') + QLatin1String(kMarkerFile));
  if (!marker.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) return false;
  marker.write(QStringLiteral("Migrated from %1 by Multai 0.5.0\n").arg(legacyRoot).toUtf8());
  marker.close();
  QFile::setPermissions(marker.fileName(), QFile::ReadOwner | QFile::WriteOwner);
  return true;
}

}  // namespace

bool DataMigration::Report::anyMigrated() const {
  for (const RootResult& root : roots) {
    if (root.migrated()) return true;
  }
  return false;
}

bool DataMigration::Report::anyFailed() const {
  for (const RootResult& root : roots) {
    if (root.failed()) return true;
  }
  return false;
}

DataMigration::Paths DataMigration::defaultPaths() {
  Paths paths;
  const QString config = QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation);
  const QString data = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation);
  QString cache = QStandardPaths::writableLocation(QStandardPaths::GenericCacheLocation);
  if (cache.isEmpty()) cache = QDir::homePath() + QStringLiteral("/.cache");

  if (!config.isEmpty()) {
    paths.oldConfigRoot = config + QStringLiteral("/ai-workspace");
    paths.newConfigRoot = config + QStringLiteral("/multai");
  }
  if (!data.isEmpty()) {
    paths.oldDataRoot = data + QStringLiteral("/ai-workspace");
    paths.newDataRoot = data + QStringLiteral("/multai");
  }
  paths.oldCacheRoot = cache + QStringLiteral("/ai-workspace");
  paths.newCacheRoot = cache + QStringLiteral("/multai");
  return paths;
}

DataMigration::Report DataMigration::run() {
  return run(defaultPaths());
}

DataMigration::Report DataMigration::run(const Paths& paths) {
  Report report;
  report.roots.append(migrateRoot(RootKind::Config, paths.oldConfigRoot, paths.newConfigRoot));
  report.roots.append(migrateRoot(RootKind::Data, paths.oldDataRoot, paths.newDataRoot));
  report.roots.append(migrateRoot(RootKind::Cache, paths.oldCacheRoot, paths.newCacheRoot));
  return report;
}

DataMigration::RootResult DataMigration::migrateRoot(RootKind kind, const QString& oldRoot,
                                                      const QString& newRoot) {
  RootResult result{kind, Status::Skipped, oldRoot, newRoot, {}};
  if (oldRoot.isEmpty() || newRoot.isEmpty() || oldRoot == newRoot) {
    result.message = QStringLiteral("no legacy location to migrate");
    return result;
  }

  const QFileInfo oldInfo(oldRoot);
  if (!oldInfo.exists()) {
    result.message = QStringLiteral("legacy location does not exist");
    return result;
  }
  if (!oldInfo.isDir()) {
    result.status = Status::Failed;
    result.message = QStringLiteral("legacy location is not a directory");
    return result;
  }

  if (QFileInfo::exists(newRoot) || QFileInfo(newRoot).isSymLink()) {
    result.message = QStringLiteral("Multai location already exists; nothing was overwritten");
    qInfo().noquote() << QStringLiteral("Multai migration (%1): skipped, %2 already exists")
                             .arg(kindName(kind), newRoot);
    return result;
  }

  const QString staging = newRoot + QStringLiteral(".migrating");
  if (QFileInfo::exists(staging) || QFileInfo(staging).isSymLink()) {
    qInfo().noquote() << QStringLiteral("Multai migration (%1): removing stale staging %2")
                             .arg(kindName(kind), staging);
    QDir(staging).removeRecursively();
  }

  if (!QDir().mkpath(QFileInfo(newRoot).absolutePath())) {
    result.status = Status::Failed;
    result.message = QStringLiteral("cannot create the destination parent directory");
    return result;
  }

  QString error;
  if (!copyTree(oldRoot, staging, &error)) {
    QDir(staging).removeRecursively();
    result.status = Status::Failed;
    result.message = error;
    qWarning().noquote() << QStringLiteral("Multai migration (%1) failed: %2")
                                .arg(kindName(kind), error);
    return result;
  }

  if (!renameTopLevel(staging, &error) || !verifyTree(oldRoot, staging, &error) ||
      !writeMarker(staging, oldRoot)) {
    QDir(staging).removeRecursively();
    result.status = Status::Failed;
    result.message = error.isEmpty() ? QStringLiteral("cannot write migration marker") : error;
    qWarning().noquote() << QStringLiteral("Multai migration (%1) failed: %2")
                                .arg(kindName(kind), result.message);
    return result;
  }

  if (!QDir().rename(staging, newRoot)) {
    QDir(staging).removeRecursively();
    result.status = Status::Failed;
    result.message = QStringLiteral("cannot move staged data into place");
    qWarning().noquote() << QStringLiteral("Multai migration (%1) failed: %2")
                                .arg(kindName(kind), result.message);
    return result;
  }

  result.status = Status::Migrated;
  result.message = QStringLiteral("migrated");
  qInfo().noquote() << QStringLiteral("Multai migration (%1): %2 -> %3")
                           .arg(kindName(kind), oldRoot, newRoot);
  return result;
}
