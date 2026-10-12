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

signals:
  void settingChanged(const QString& key);

private:
  explicit SettingsManager(QObject* parent = nullptr);
  ~SettingsManager() override = default;

  QSettings m_settings;
};
