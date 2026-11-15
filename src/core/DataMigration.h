#pragma once

#include <QList>
#include <QString>

class DataMigration {
 public:
  enum class RootKind { Config, Data, Cache };
  enum class Status { Skipped, Migrated, Failed };

  struct Paths {
    QString oldConfigRoot;
    QString newConfigRoot;
    QString oldDataRoot;
    QString newDataRoot;
    QString oldCacheRoot;
    QString newCacheRoot;
  };

  struct RootResult {
    RootKind kind;
    Status status;
    QString oldPath;
    QString newPath;
    QString message;

    bool migrated() const { return status == Status::Migrated; }
    bool failed() const { return status == Status::Failed; }
  };

  struct Report {
    QList<RootResult> roots;

    bool anyMigrated() const;
    bool anyFailed() const;
  };

  static Paths defaultPaths();
  static Report run();
  static Report run(const Paths& paths);

 private:
  static RootResult migrateRoot(RootKind kind, const QString& oldRoot, const QString& newRoot);
};
