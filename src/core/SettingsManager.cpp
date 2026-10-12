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
