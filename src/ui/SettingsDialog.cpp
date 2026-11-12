#include "SettingsDialog.h"
#include "SettingsManager.h"

#include <QCheckBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLabel>
#include <QSpinBox>
#include <QVBoxLayout>

SettingsDialog::SettingsDialog(QWidget* parent)
  : ThemedDialog(parent)
  , m_themeCombo(nullptr)
  , m_automationCheck(nullptr)
  , m_freezeSpin(nullptr)
  , m_hibernateSpin(nullptr)
  , m_respectRecommendedCheck(nullptr)
  , m_memoryPressureCheck(nullptr)
  , m_memoryThresholdSpin(nullptr) {
  setWindowTitle("Settings");
  setMinimumWidth(460);

  QVBoxLayout* mainLayout = new QVBoxLayout(this);

  QLabel* appearanceLabel = new QLabel("Appearance", this);
  QFont sectionFont = appearanceLabel->font();
  sectionFont.setBold(true);
  appearanceLabel->setFont(sectionFont);
  mainLayout->addWidget(appearanceLabel);

  QFormLayout* appearanceLayout = new QFormLayout();
  m_themeCombo = new QComboBox(this);
  m_themeCombo->setObjectName("themeCombo");
  m_themeCombo->addItem("System", "system");
  m_themeCombo->addItem("Light", "light");
  m_themeCombo->addItem("Dark", "dark");
  m_themeCombo->setToolTip("Select the application colour theme");
  appearanceLayout->addRow("Theme:", m_themeCombo);
  mainLayout->addLayout(appearanceLayout);

  QLabel* lifecycleLabel = new QLabel("Lifecycle", this);
  lifecycleLabel->setFont(sectionFont);
  mainLayout->addWidget(lifecycleLabel);

  QFormLayout* lifecycleLayout = new QFormLayout();

  m_automationCheck = new QCheckBox("Enable automatic freezing and hibernation", this);
  m_automationCheck->setObjectName("automationCheck");
  m_automationCheck->setToolTip("Suspend inactive tabs to reduce resource usage");
  lifecycleLayout->addRow(m_automationCheck);

  m_freezeSpin = new QSpinBox(this);
  m_freezeSpin->setObjectName("freezeSpin");
  m_freezeSpin->setRange(1, 240);
  m_freezeSpin->setSuffix(" min");
  m_freezeSpin->setToolTip("Delay before an inactive tab is frozen");
  lifecycleLayout->addRow("Freeze after:", m_freezeSpin);

  m_hibernateSpin = new QSpinBox(this);
  m_hibernateSpin->setObjectName("hibernateSpin");
  m_hibernateSpin->setRange(1, 600);
  m_hibernateSpin->setSuffix(" min");
  m_hibernateSpin->setToolTip("Delay before an inactive tab is hibernated");
  lifecycleLayout->addRow("Hibernate after:", m_hibernateSpin);

  m_respectRecommendedCheck = new QCheckBox("Respect Qt recommendedState", this);
  m_respectRecommendedCheck->setObjectName("respectRecommendedCheck");
  m_respectRecommendedCheck->setToolTip(
      "Only suspend tabs when Qt recommends it. Disabling may stop background audio "
      "or discard unsaved form input in inactive tabs.");
  lifecycleLayout->addRow(m_respectRecommendedCheck);

  m_memoryPressureCheck = new QCheckBox("Hibernate on memory pressure", this);
  m_memoryPressureCheck->setObjectName("memoryPressureCheck");
  m_memoryPressureCheck->setToolTip(
      "Hibernate the least recently used inactive tab when available memory is low");
  lifecycleLayout->addRow(m_memoryPressureCheck);

  m_memoryThresholdSpin = new QSpinBox(this);
  m_memoryThresholdSpin->setObjectName("memoryThresholdSpin");
  m_memoryThresholdSpin->setRange(1, 90);
  m_memoryThresholdSpin->setSuffix(" %");
  m_memoryThresholdSpin->setToolTip("Available-memory threshold that triggers hibernation");
  lifecycleLayout->addRow("Pressure threshold:", m_memoryThresholdSpin);

  m_memoryRecoveryThresholdSpin = new QSpinBox(this);
  m_memoryRecoveryThresholdSpin->setObjectName("memoryRecoveryThresholdSpin");
  m_memoryRecoveryThresholdSpin->setRange(1, 90);
  m_memoryRecoveryThresholdSpin->setSuffix(" %");
  m_memoryRecoveryThresholdSpin->setToolTip(
      "Available-memory level above which pressure is considered relieved (hysteresis)");
  lifecycleLayout->addRow("Recovery threshold:", m_memoryRecoveryThresholdSpin);

  mainLayout->addLayout(lifecycleLayout);
  mainLayout->addStretch();

  QDialogButtonBox* buttonBox = new QDialogButtonBox(
      QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
  connect(buttonBox, &QDialogButtonBox::accepted, this, &SettingsDialog::apply);
  connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
  mainLayout->addWidget(buttonBox);

  load();
}

void SettingsDialog::load() {
  SettingsManager* settings = SettingsManager::instance();

  const int themeIndex = m_themeCombo->findData(settings->theme());
  m_themeCombo->setCurrentIndex(themeIndex >= 0 ? themeIndex : 0);

  m_automationCheck->setChecked(settings->lifecycleAutomationEnabled());
  m_freezeSpin->setValue(qMax(1, settings->freezeDelayMs() / 60000));
  m_hibernateSpin->setValue(qMax(1, settings->hibernateDelayMs() / 60000));
  m_respectRecommendedCheck->setChecked(settings->respectRecommendedState());
  m_memoryPressureCheck->setChecked(settings->memoryPressureEnabled());
  m_memoryThresholdSpin->setValue(settings->memoryPressureThresholdPercent());
  m_memoryRecoveryThresholdSpin->setValue(settings->memoryPressureRecoveryThresholdPercent());
}

void SettingsDialog::apply() {
  SettingsManager* settings = SettingsManager::instance();

  settings->setTheme(m_themeCombo->currentData().toString());
  settings->setLifecycleAutomationEnabled(m_automationCheck->isChecked());
  settings->setFreezeDelayMs(m_freezeSpin->value() * 60000);
  settings->setHibernateDelayMs(m_hibernateSpin->value() * 60000);
  settings->setRespectRecommendedState(m_respectRecommendedCheck->isChecked());
  settings->setMemoryPressureEnabled(m_memoryPressureCheck->isChecked());
  settings->setMemoryPressureThresholdPercent(m_memoryThresholdSpin->value());
  settings->setMemoryPressureRecoveryThresholdPercent(m_memoryRecoveryThresholdSpin->value());

  accept();
}
