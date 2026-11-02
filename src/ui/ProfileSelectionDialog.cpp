#include "ProfileSelectionDialog.h"
#include "Profile.h"
#include "profiles/ProfileManager.h"

#include <QComboBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>

ProfileSelectionDialog::ProfileSelectionDialog(const QString& providerId,
                                               const QString& providerName, QWidget* parent)
  : QDialog(parent)
  , m_providerId(providerId)
  , m_accountCombo(nullptr)
  , m_newAccountEdit(nullptr)
  , m_okButton(nullptr) {
  setWindowTitle("Select Account");
  setMinimumWidth(420);

  QVBoxLayout* mainLayout = new QVBoxLayout(this);

  QLabel* titleLabel = new QLabel(QString("Choose an account for %1").arg(providerName), this);
  titleLabel->setWordWrap(true);
  mainLayout->addWidget(titleLabel);

  QFormLayout* formLayout = new QFormLayout();

  m_accountCombo = new QComboBox(this);
  m_accountCombo->setToolTip("Existing accounts for this provider");
  connect(m_accountCombo, &QComboBox::currentIndexChanged, this,
          &ProfileSelectionDialog::updateOkState);
  formLayout->addRow("Account:", m_accountCombo);

  m_newAccountEdit = new QLineEdit(this);
  m_newAccountEdit->setPlaceholderText("New account name");
  m_newAccountEdit->setToolTip("Create an additional isolated account for this provider");
  formLayout->addRow("Add account:", m_newAccountEdit);

  mainLayout->addLayout(formLayout);

  QPushButton* createButton = new QPushButton("Create Account", this);
  createButton->setToolTip("Create a new isolated profile for this provider");
  connect(createButton, &QPushButton::clicked, this, &ProfileSelectionDialog::onCreateAccount);
  mainLayout->addWidget(createButton);

  QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
  m_okButton = buttonBox->button(QDialogButtonBox::Ok);
  m_okButton->setText("Open");
  m_okButton->setToolTip("Open a tab using the selected account");
  connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
  connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
  mainLayout->addWidget(buttonBox);

  reloadAccounts();
}

void ProfileSelectionDialog::reloadAccounts() {
  m_accountCombo->clear();

  QList<Profile*> profiles = ProfileManager::instance()->profilesForProvider(m_providerId);
  if (profiles.isEmpty()) {
    ProfileManager::instance()->getOrCreateProfile(m_providerId, "default");
    profiles = ProfileManager::instance()->profilesForProvider(m_providerId);
  }

  for (Profile* profile : profiles) {
    m_accountCombo->addItem(QString("%1 (%2)").arg(profile->name(), profile->accountId()),
                            profile->accountId());
  }

  updateOkState();
}

void ProfileSelectionDialog::onCreateAccount() {
  const QString name = m_newAccountEdit->text().trimmed();
  if (name.isEmpty()) {
    return;
  }

  Profile* profile = ProfileManager::instance()->createProfile(m_providerId, name);
  if (!profile) {
    return;
  }

  m_newAccountEdit->clear();
  reloadAccounts();

  const int index = m_accountCombo->findData(profile->accountId());
  if (index >= 0) {
    m_accountCombo->setCurrentIndex(index);
  }
}

void ProfileSelectionDialog::updateOkState() {
  m_okButton->setEnabled(!m_accountCombo->currentData().toString().isEmpty());
}

QString ProfileSelectionDialog::selectedAccountId() const {
  return m_accountCombo->currentData().toString();
}
