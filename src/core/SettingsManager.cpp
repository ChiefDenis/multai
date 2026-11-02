#include "SettingsManager.h"

SettingsManager::SettingsManager(QObject* parent)
  : QObject(parent)
  , m_settings("ai-workspace", "AI Workspace") {
}

SettingsManager* SettingsManager::instance() {
  static SettingsManager instance;
  return &instance;
}

QVariant SettingsManager::value(const QString& key, const QVariant& defaultValue) const {
  return m_settings.value(key, defaultValue);
}

void SettingsManager::setValue(const QString& key, const QVariant& value) {
  m_settings.setValue(key, value);
  m_settings.sync();
  emit settingChanged(key);
}

void SettingsManager::reset(const QString& key) {
  m_settings.remove(key);
  m_settings.sync();
  emit settingChanged(key);
}

void SettingsManager::resetAll() {
  m_settings.clear();
  m_settings.sync();
}

QString SettingsManager::theme() const {
  return value("appearance/theme", "system").toString();
}

void SettingsManager::setTheme(const QString& theme) {
  setValue("appearance/theme", theme);
}

QByteArray SettingsManager::windowGeometry() const {
  return value("window/geometry").toByteArray();
}

void SettingsManager::setWindowGeometry(const QByteArray& geometry) {
  setValue("window/geometry", geometry);
}

QByteArray SettingsManager::windowState() const {
  return value("window/state").toByteArray();
}

void SettingsManager::setWindowState(const QByteArray& state) {
  setValue("window/state", state);
}

bool SettingsManager::lifecycleAutomationEnabled() const {
  return value("lifecycle/automationEnabled", true).toBool();
}

void SettingsManager::setLifecycleAutomationEnabled(bool enabled) {
  setValue("lifecycle/automationEnabled", enabled);
}

int SettingsManager::freezeDelayMs() const {
  return value("lifecycle/freezeDelayMs", 5 * 60 * 1000).toInt();
}

void SettingsManager::setFreezeDelayMs(int msecs) {
  setValue("lifecycle/freezeDelayMs", msecs);
}

int SettingsManager::hibernateDelayMs() const {
  return value("lifecycle/hibernateDelayMs", 30 * 60 * 1000).toInt();
}

void SettingsManager::setHibernateDelayMs(int msecs) {
  setValue("lifecycle/hibernateDelayMs", msecs);
}

bool SettingsManager::respectRecommendedState() const {
  return value("lifecycle/respectRecommendedState", true).toBool();
}

void SettingsManager::setRespectRecommendedState(bool respect) {
  setValue("lifecycle/respectRecommendedState", respect);
}

bool SettingsManager::memoryPressureEnabled() const {
  return value("lifecycle/memoryPressureEnabled", true).toBool();
}

void SettingsManager::setMemoryPressureEnabled(bool enabled) {
  setValue("lifecycle/memoryPressureEnabled", enabled);
}

int SettingsManager::memoryPressureThresholdPercent() const {
  return value("lifecycle/memoryPressureThresholdPercent", 15).toInt();
}

void SettingsManager::setMemoryPressureThresholdPercent(int percent) {
  setValue("lifecycle/memoryPressureThresholdPercent", percent);
}

QString SettingsManager::profileName(const QString& fullId) const {
  return value("profiles/" + fullId + "/name").toString();
}

void SettingsManager::setProfileName(const QString& fullId, const QString& name) {
  setValue("profiles/" + fullId + "/name", name);
}

void SettingsManager::removeProfileName(const QString& fullId) {
  reset("profiles/" + fullId + "/name");
}
