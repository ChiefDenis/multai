#include "TestClock.h"

TestClock::TestClock(QObject* parent)
  : QObject(parent)
  , m_elapsed(0) {
}

TestClock* TestClock::instance() {
  static TestClock instance;
  return &instance;
}

qint64 TestClock::elapsed() const {
  return m_elapsed;
}

void TestClock::advance(qint64 msecs) {
  m_elapsed += msecs;
  emit timeAdvanced(msecs);
}

void TestClock::reset() {
  m_elapsed = 0;
}

bool TestClock::isTestMode() {
  return s_testMode;
}

void TestClock::setTestMode(bool enabled) {
  s_testMode = enabled;
}

bool TestClock::s_testMode = false;
