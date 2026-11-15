#include <QtTest/QtTest>

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QSettings>
#include <QTemporaryDir>

#include "DataMigration.h"

namespace {

void writeFile(const QString& path, const QByteArray& content) {
  QVERIFY(QDir().mkpath(QFileInfo(path).absolutePath()));
  QFile file(path);
  QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Truncate));
  file.write(content);
}

QByteArray readFile(const QString& path) {
  QFile file(path);
  if (!file.open(QIODevice::ReadOnly)) return {};
  return file.readAll();
}

DataMigration::Paths makePaths(const QString& base) {
  DataMigration::Paths paths;
  paths.oldConfigRoot = base + "/old-config/ai-workspace";
  paths.newConfigRoot = base + "/new-config/multai";
  paths.oldDataRoot = base + "/old-data/ai-workspace";
  paths.newDataRoot = base + "/new-data/multai";
  paths.oldCacheRoot = base + "/old-cache/ai-workspace";
  paths.newCacheRoot = base + "/new-cache/multai";
  return paths;
}

void populateLegacyRoots(const DataMigration::Paths& paths) {
  writeFile(paths.oldConfigRoot + "/AI Workspace.conf",
            "[workspace]\nopenTabs=deepseek\nlastProvider=qwen\n");
  writeFile(paths.oldConfigRoot + "/AI Workspace/profiles/deepseek:default/storage/Cookies",
            QByteArray("cookie-bytes", 12));
  writeFile(paths.oldConfigRoot + "/AI Workspace/profiles/deepseek:default/.hidden", "hidden");
  writeFile(paths.oldDataRoot +
                "/AI Workspace/QtWebEngine/deepseek-default/Local Storage/leveldb/000001.log",
            QByteArray("\x01\x02\x03", 3));
  writeFile(paths.oldCacheRoot + "/AI Workspace/qtpipelinecache-x86_64/data", "cache");
}

}  // namespace

class TestMigration : public QObject {
  Q_OBJECT
 private slots:
  void testMigratesLegacyRoots();
  void testIdempotentAndKeepsMigratedData();
  void testExistingMultaiDataIsMergedWithoutOverwriting();
  void testMarkerPreventsRepeatedMerge();
  void testStaleStagingIsReplaced();
  void testMissingLegacyRootsAreSkipped();
  void testRootsAreIndependent();
};

void TestMigration::testMigratesLegacyRoots() {
  QTemporaryDir temp;
  QVERIFY(temp.isValid());
  const DataMigration::Paths paths = makePaths(temp.path());
  populateLegacyRoots(paths);

  const DataMigration::Report report = DataMigration::run(paths);
  QCOMPARE(report.roots.size(), 3);
  for (const DataMigration::RootResult& root : report.roots) {
    QVERIFY2(root.migrated(), qPrintable(root.message));
  }
  QVERIFY(report.anyMigrated());
  QVERIFY(!report.anyFailed());

  QSettings migrated(paths.newConfigRoot + "/Multai.conf", QSettings::IniFormat);
  QCOMPARE(migrated.value("workspace/openTabs").toString(), QString("deepseek"));
  QCOMPARE(migrated.value("workspace/lastProvider").toString(), QString("qwen"));
  QCOMPARE(readFile(paths.newConfigRoot + "/Multai/profiles/deepseek:default/storage/Cookies"),
           QByteArray("cookie-bytes", 12));
  QCOMPARE(readFile(paths.newConfigRoot + "/Multai/profiles/deepseek:default/.hidden"),
           QByteArray("hidden"));
  QCOMPARE(readFile(paths.newDataRoot +
                    "/Multai/QtWebEngine/deepseek-default/Local Storage/leveldb/000001.log"),
           QByteArray("\x01\x02\x03", 3));
  QCOMPARE(readFile(paths.newCacheRoot + "/Multai/qtpipelinecache-x86_64/data"),
           QByteArray("cache"));

  QVERIFY(!QFileInfo::exists(paths.newConfigRoot + "/AI Workspace.conf"));
  QVERIFY(!QFileInfo::exists(paths.newConfigRoot + "/AI Workspace"));
  QVERIFY(QFileInfo::exists(paths.newConfigRoot + "/.migration-complete"));
  QVERIFY(QFileInfo::exists(paths.newDataRoot + "/.migration-complete"));
  QVERIFY(QFileInfo::exists(paths.newCacheRoot + "/.migration-complete"));

  // The legacy tree is retained for rollback.
  QVERIFY(QFileInfo::exists(paths.oldConfigRoot + "/AI Workspace.conf"));
  QVERIFY(QFileInfo::exists(
      paths.oldConfigRoot + "/AI Workspace/profiles/deepseek:default/storage/Cookies"));
  QVERIFY(QFileInfo::exists(paths.oldDataRoot + "/AI Workspace"));
  QCOMPARE(readFile(paths.oldConfigRoot + "/AI Workspace.conf"),
           QByteArray("[workspace]\nopenTabs=deepseek\nlastProvider=qwen\n"));
}

void TestMigration::testIdempotentAndKeepsMigratedData() {
  QTemporaryDir temp;
  QVERIFY(temp.isValid());
  const DataMigration::Paths paths = makePaths(temp.path());
  populateLegacyRoots(paths);

  QVERIFY(DataMigration::run(paths).anyMigrated());

  writeFile(paths.newConfigRoot + "/Multai.conf", "[workspace]\nopenTabs=kimi\n");
  const DataMigration::Report second = DataMigration::run(paths);
  QVERIFY(!second.anyMigrated());
  QVERIFY(!second.anyFailed());
  for (const DataMigration::RootResult& root : second.roots) {
    QCOMPARE(static_cast<int>(root.status),
             static_cast<int>(DataMigration::Status::Skipped));
  }
  QSettings kept(paths.newConfigRoot + "/Multai.conf", QSettings::IniFormat);
  QCOMPARE(kept.value("workspace/openTabs").toString(), QString("kimi"));
  QVERIFY(QFileInfo::exists(paths.oldConfigRoot + "/AI Workspace.conf"));
  QVERIFY(!QFileInfo::exists(paths.newConfigRoot + ".migrating"));
}

void TestMigration::testExistingMultaiDataIsMergedWithoutOverwriting() {
  QTemporaryDir temp;
  QVERIFY(temp.isValid());
  const DataMigration::Paths paths = makePaths(temp.path());
  populateLegacyRoots(paths);
  writeFile(paths.newConfigRoot + "/Multai.conf", "[workspace]\nopenTabs=kimi\ntheme=dark\n");
  writeFile(paths.newConfigRoot + "/Multai/profiles/existing:default/keep", "keep");

  const DataMigration::Report report = DataMigration::run(paths);

  QCOMPARE(static_cast<int>(report.roots.at(0).status),
           static_cast<int>(DataMigration::Status::Migrated));
  QSettings merged(paths.newConfigRoot + "/Multai.conf", QSettings::IniFormat);
  QCOMPARE(merged.value("workspace/openTabs").toString(), QString("kimi"));
  QCOMPARE(merged.value("workspace/theme").toString(), QString("dark"));
  QCOMPARE(merged.value("workspace/lastProvider").toString(), QString("qwen"));
  QCOMPARE(readFile(paths.newConfigRoot + "/Multai/profiles/existing:default/keep"),
           QByteArray("keep"));
  QCOMPARE(readFile(paths.newConfigRoot + "/Multai/profiles/deepseek:default/storage/Cookies"),
           QByteArray("cookie-bytes", 12));
  QVERIFY(QFileInfo::exists(paths.newConfigRoot + "/.migration-complete"));
  QVERIFY(!QFileInfo::exists(paths.newConfigRoot + ".migrating"));
  QVERIFY(QFileInfo::exists(paths.oldConfigRoot + "/AI Workspace.conf"));
}

void TestMigration::testMarkerPreventsRepeatedMerge() {
  QTemporaryDir temp;
  QVERIFY(temp.isValid());
  const DataMigration::Paths paths = makePaths(temp.path());
  populateLegacyRoots(paths);
  QVERIFY(DataMigration::run(paths).anyMigrated());

  writeFile(paths.oldConfigRoot + "/AI Workspace/profiles/late:default/keep", "late");
  const DataMigration::Report second = DataMigration::run(paths);
  QVERIFY(!second.anyMigrated());
  QVERIFY(!second.anyFailed());
  QVERIFY(!QFileInfo::exists(paths.newConfigRoot + "/Multai/profiles/late:default/keep"));
  QVERIFY(QFileInfo::exists(paths.oldConfigRoot + "/AI Workspace/profiles/late:default/keep"));
}

void TestMigration::testStaleStagingIsReplaced() {
  QTemporaryDir temp;
  QVERIFY(temp.isValid());
  const DataMigration::Paths paths = makePaths(temp.path());
  populateLegacyRoots(paths);
  writeFile(paths.newConfigRoot + ".migrating/partial-copy", "stale");

  const DataMigration::Report report = DataMigration::run(paths);
  QVERIFY(report.anyMigrated());
  QVERIFY(!QFileInfo::exists(paths.newConfigRoot + ".migrating/partial-copy"));
  QVERIFY(!QFileInfo::exists(paths.newConfigRoot + ".migrating"));
  QSettings migrated(paths.newConfigRoot + "/Multai.conf", QSettings::IniFormat);
  QCOMPARE(migrated.value("workspace/openTabs").toString(), QString("deepseek"));
}

void TestMigration::testMissingLegacyRootsAreSkipped() {
  QTemporaryDir temp;
  QVERIFY(temp.isValid());
  const DataMigration::Paths paths = makePaths(temp.path());

  const DataMigration::Report report = DataMigration::run(paths);
  QVERIFY(!report.anyMigrated());
  QVERIFY(!report.anyFailed());
  for (const DataMigration::RootResult& root : report.roots) {
    QCOMPARE(static_cast<int>(root.status),
             static_cast<int>(DataMigration::Status::Skipped));
  }
  QVERIFY(!QFileInfo::exists(paths.newConfigRoot));
  QVERIFY(!QFileInfo::exists(paths.newDataRoot));
  QVERIFY(!QFileInfo::exists(paths.newCacheRoot));
}

void TestMigration::testRootsAreIndependent() {
  QTemporaryDir temp;
  QVERIFY(temp.isValid());
  const DataMigration::Paths paths = makePaths(temp.path());
  populateLegacyRoots(paths);
  writeFile(paths.newDataRoot + "/Multai/existing", "existing");

  const DataMigration::Report report = DataMigration::run(paths);
  QVERIFY(report.roots.at(0).migrated());
  QCOMPARE(static_cast<int>(report.roots.at(1).status),
           static_cast<int>(DataMigration::Status::Migrated));
  QVERIFY(report.roots.at(2).migrated());
  QCOMPARE(readFile(paths.newDataRoot + "/Multai/existing"), QByteArray("existing"));
  QCOMPARE(readFile(paths.newDataRoot +
                    "/Multai/QtWebEngine/deepseek-default/Local Storage/leveldb/000001.log"),
           QByteArray("\x01\x02\x03", 3));
  QVERIFY(QFileInfo::exists(paths.oldDataRoot + "/AI Workspace"));
}

QTEST_GUILESS_MAIN(TestMigration)
#include "test_migration.moc"
