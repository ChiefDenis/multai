#pragma once

#include <QObject>
#include <QElapsedTimer>

class TestClock : public QObject {
  Q_OBJECT
public:
  static TestClock* instance();

  qint64 elapsed() const;
  void advance(qint64 msecs);
  void reset();

  static bool isTestMode();
  static void setTestMode(bool enabled);

signals:
  void timeAdvanced(qint64 msecs);

private:
  explicit TestClock(QObject* parent = nullptr);
  ~TestClock() override = default;

  qint64 m_elapsed;
  static bool s_testMode;
};
