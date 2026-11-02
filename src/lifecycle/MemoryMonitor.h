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

  int availablePercent() const;

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
  int m_lastAvailablePercent;
};
