#include "MemoryMonitor.h"
#include "SettingsManager.h"

#include <QFile>
#include <QTimer>

MemoryMonitor::MemoryMonitor(QObject* parent)
  : QObject(parent)
  , m_timer(new QTimer(this))
  , m_intervalMs(15000)
  , m_thresholdPercent(15)
  , m_recoveryThresholdPercent(20)
  , m_lastAvailablePercent(-1)
  , m_inPressureState(false)
  , m_memoryOverrideEnabled(false)
  , m_overrideTotalBytes(0)
  , m_overrideAvailableBytes(0) {
  m_timer->setInterval(m_intervalMs);
  m_timer->setSingleShot(false);
  connect(m_timer, &QTimer::timeout, this, &MemoryMonitor::check);
}

bool MemoryMonitor::parseMemInfo(const QString& content, quint64* totalBytes, quint64* availableBytes) {
  quint64 totalKb = 0;
  quint64 availableKb = 0;
  quint64 freeKb = 0;
  quint64 buffersKb = 0;
  quint64 cachedKb = 0;

  for (const QString& line : content.split('\n')) {
    const QString key = line.section(':', 0, 0).trimmed();
    if (key.isEmpty()) {
      continue;
    }

    bool ok = false;
    const quint64 valueKb = line.section(':', 1).trimmed().section(' ', 0, 0).toULongLong(&ok);
    if (!ok) {
      continue;
    }

    if (key == "MemTotal") {
      totalKb = valueKb;
    } else if (key == "MemAvailable") {
      availableKb = valueKb;
    } else if (key == "MemFree") {
      freeKb = valueKb;
    } else if (key == "Buffers") {
      buffersKb = valueKb;
    } else if (key == "Cached") {
      cachedKb = valueKb;
    }
  }

  if (totalKb == 0) {
    return false;
  }

  if (availableKb == 0) {
    availableKb = freeKb + buffersKb + cachedKb;
  }

  if (totalBytes) {
    *totalBytes = totalKb * 1024ULL;
  }
  if (availableBytes) {
    *availableBytes = availableKb * 1024ULL;
  }
  return true;
}

bool MemoryMonitor::readMemoryInfo(quint64* totalBytes, quint64* availableBytes) {
  QFile file("/proc/meminfo");
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    return false;
  }
  return parseMemInfo(QString::fromUtf8(file.readAll()), totalBytes, availableBytes);
}

bool MemoryMonitor::parsePressure(const QString& content, double* someAvg10) {
  for (const QString& line : content.split('\n')) {
    if (!line.startsWith("some")) {
      continue;
    }

    bool ok = false;
    const double avg = line.section('=', 1, 1).trimmed().section(' ', 0, 0).toDouble(&ok);
    if (!ok) {
      continue;
    }

    if (someAvg10) {
      *someAvg10 = avg;
    }
    return true;
  }
  return false;
}

void MemoryMonitor::start(int intervalMs) {
  SettingsManager* settings = SettingsManager::instance();

  if (intervalMs > 0) {
    m_intervalMs = intervalMs;
  }
  m_thresholdPercent = qBound(1, settings->memoryPressureThresholdPercent(), 90);
  m_recoveryThresholdPercent = qBound(1, settings->memoryPressureRecoveryThresholdPercent(), 90);
  if (m_recoveryThresholdPercent <= m_thresholdPercent) {
    m_recoveryThresholdPercent = m_thresholdPercent + 1;
    if (m_recoveryThresholdPercent > 90) {
      m_recoveryThresholdPercent = 90;
    }
  }
  m_timer->setInterval(m_intervalMs);

  if (!settings->memoryPressureEnabled()) {
    m_timer->stop();
    return;
  }

  m_timer->start();
  check();
}

void MemoryMonitor::stop() {
  m_timer->stop();
  m_inPressureState = false;
}

bool MemoryMonitor::isRunning() const {
  return m_timer->isActive();
}

int MemoryMonitor::pressureThresholdPercent() const {
  return m_thresholdPercent;
}

void MemoryMonitor::setPressureThresholdPercent(int percent) {
  m_thresholdPercent = qBound(1, percent, 90);
}

int MemoryMonitor::recoveryThresholdPercent() const {
  return m_recoveryThresholdPercent;
}

void MemoryMonitor::setRecoveryThresholdPercent(int percent) {
  m_recoveryThresholdPercent = qBound(1, percent, 90);
  if (m_recoveryThresholdPercent <= m_thresholdPercent) {
    m_recoveryThresholdPercent = m_thresholdPercent + 1;
    if (m_recoveryThresholdPercent > 90) {
      m_recoveryThresholdPercent = 90;
    }
  }
}

int MemoryMonitor::availablePercent() const {
  return m_lastAvailablePercent;
}

bool MemoryMonitor::isInPressureState() const {
  return m_inPressureState;
}

void MemoryMonitor::setMemoryOverride(quint64 totalBytes, quint64 availableBytes) {
  m_memoryOverrideEnabled = true;
  m_overrideTotalBytes = totalBytes;
  m_overrideAvailableBytes = availableBytes;
}

void MemoryMonitor::clearMemoryOverride() {
  m_memoryOverrideEnabled = false;
}

void MemoryMonitor::check() {
  quint64 total = 0;
  quint64 available = 0;
  if (m_memoryOverrideEnabled) {
    total = m_overrideTotalBytes;
    available = m_overrideAvailableBytes;
  } else if (!readMemoryInfo(&total, &available) || total == 0) {
    return;
  }

  const int percent = static_cast<int>((available * 100) / total);
  m_lastAvailablePercent = percent;

  if (m_inPressureState) {
    // Currently in pressure; check for recovery
    if (percent > m_recoveryThresholdPercent) {
      m_inPressureState = false;
      emit memoryAvailable(percent);
    }
  } else {
    // Currently normal; check for pressure entry
    if (percent <= m_thresholdPercent) {
      m_inPressureState = true;
      emit memoryPressureDetected(percent);
    }
  }
}
