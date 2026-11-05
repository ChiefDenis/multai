#pragma once

#include <QObject>
#include <QString>

class QTimer;

class MemoryMonitor : public QObject {
  Q_OBJECT
public:
  explicit MemoryMonitor(QObject* parent = nullptr);
  ~MemoryMonitor() override = default;

  static bool parseMemInfo(const QString& content, quint64* totalBytes, quint64* availableBytes);
  static bool readMemoryInfo(quint64* totalBytes, quint64* availableBytes);
  static bool parsePressure(const QString& content, double* someAvg10);

  void start(int intervalMs = 15000);
  void stop();
  bool isRunning() const;

  int pressureThresholdPercent() const;
  void setPressureThresholdPercent(int percent);

  int recoveryThresholdPercent() const;
  void setRecoveryThresholdPercent(int percent);

  int availablePercent() const;
  bool isInPressureState() const;

  // Testing seam: override /proc/meminfo values
  void setMemoryOverride(quint64 totalBytes, quint64 availableBytes);
  void clearMemoryOverride();

signals:
  void memoryPressureDetected(int availablePercent);
  void memoryAvailable(int availablePercent);

private slots:
  void check();

private:
  void reloadSettings();

  QTimer* m_timer;
  int m_intervalMs;
  int m_thresholdPercent;
  int m_recoveryThresholdPercent;
  int m_lastAvailablePercent;
  bool m_inPressureState;

  bool m_memoryOverrideEnabled;
  quint64 m_overrideTotalBytes;
  quint64 m_overrideAvailableBytes;
};
