#pragma once

#include <QObject>
#include <QSettings>

class SettingsManager : public QObject {
  Q_OBJECT
public:
  static SettingsManager* instance();

  QVariant value(const QString& key, const QVariant& defaultValue = QVariant()) const;
  void setValue(const QString& key, const QVariant& value);
  void reset(const QString& key);
  void resetAll();

  QString theme() const;
  void setTheme(const QString& theme);

  QByteArray windowGeometry() const;
  void setWindowGeometry(const QByteArray& geometry);

  QByteArray windowState() const;
  void setWindowState(const QByteArray& state);

  bool lifecycleAutomationEnabled() const;
  void setLifecycleAutomationEnabled(bool enabled);

  int freezeDelayMs() const;
  void setFreezeDelayMs(int msecs);

  int hibernateDelayMs() const;
  void setHibernateDelayMs(int msecs);

  bool respectRecommendedState() const;
  void setRespectRecommendedState(bool respect);

  bool memoryPressureEnabled() const;
  void setMemoryPressureEnabled(bool enabled);

  int memoryPressureThresholdPercent() const;
  void setMemoryPressureThresholdPercent(int percent);

  int memoryPressureRecoveryThresholdPercent() const;
  void setMemoryPressureRecoveryThresholdPercent(int percent);

  QString profileName(const QString& fullId) const;
  void setProfileName(const QString& fullId, const QString& name);
  void removeProfileName(const QString& fullId);

signals:
  void settingChanged(const QString& key);

private:
  explicit SettingsManager(QObject* parent = nullptr);
  ~SettingsManager() override = default;

  QSettings m_settings;
};
