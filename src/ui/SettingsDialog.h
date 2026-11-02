#pragma once

#include <QDialog>

class QCheckBox;
class QComboBox;
class QSpinBox;

class SettingsDialog : public QDialog {
  Q_OBJECT
public:
  explicit SettingsDialog(QWidget* parent = nullptr);
  ~SettingsDialog() override = default;

private slots:
  void apply();

private:
  void load();

  QComboBox* m_themeCombo;
  QCheckBox* m_automationCheck;
  QSpinBox* m_freezeSpin;
  QSpinBox* m_hibernateSpin;
  QCheckBox* m_respectRecommendedCheck;
  QCheckBox* m_memoryPressureCheck;
  QSpinBox* m_memoryThresholdSpin;
};
