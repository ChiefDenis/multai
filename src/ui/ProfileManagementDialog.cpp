#include "ProfileManagementDialog.h"
#include "Profile.h"
#include "profiles/ProfileManager.h"
#include "providers/Provider.h"
#include "providers/ProviderRegistry.h"

#include <QComboBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>

ProfileManagementDialog::ProfileManagementDialog(QWidget* parent)
  : QDialog(parent)
  , m_providerCombo(nullptr)
  , m_profileList(nullptr)
  , m_nameEdit(nullptr)
  , m_renameButton(nullptr)
  , m_deleteButton(nullptr) {
  setWindowTitle("Profiles");
  setMinimumSize(520, 420);

  QVBoxLayout* mainLayout = new QVBoxLayout(this);

  QLabel* titleLabel = new QLabel("Manage provider accounts", this);
  QFont titleFont = titleLabel->font();
  titleFont.setPointSize(14);
  titleFont.setBold(true);
  titleLabel->setFont(titleFont);
  mainLayout->addWidget(titleLabel);

  QFormLayout* formLayout = new QFormLayout();
  m_providerCombo = new QComboBox(this);
  for (Provider* provider : ProviderRegistry::instance()->providers()) {
    m_providerCombo->addItem(provider->displayName(), provider->id());
  }
  formLayout->addRow("Provider:", m_providerCombo);
  mainLayout->addLayout(formLayout);

  m_profileList = new QListWidget(this);
  m_profileList->setAlternatingRowColors(true);
  m_profileList->setToolTip("Select an account to rename or delete");
  connect(m_profileList, &QListWidget::itemSelectionChanged, this, [this]() {
    QListWidgetItem* item = m_profileList->currentItem();
    if (item) {
      m_nameEdit->setText(item->data(Qt::UserRole + 1).toString());
    }
    updateButtons();
  });
  mainLayout->addWidget(m_profileList);

  QHBoxLayout* actionLayout = new QHBoxLayout();
  m_nameEdit = new QLineEdit(this);
  m_nameEdit->setPlaceholderText("Account name");
  m_nameEdit->setToolTip("Name used when creating or renaming an account");
  actionLayout->addWidget(m_nameEdit);

  QPushButton* createButton = new QPushButton("Create", this);
  createButton->setToolTip("Create a new account for the selected provider");
  connect(createButton, &QPushButton::clicked, this, &ProfileManagementDialog::onCreate);
  actionLayout->addWidget(createButton);

  m_renameButton = new QPushButton("Rename", this);
  m_renameButton->setToolTip("Rename the selected account");
  connect(m_renameButton, &QPushButton::clicked, this, &ProfileManagementDialog::onRename);
  actionLayout->addWidget(m_renameButton);

  m_deleteButton = new QPushButton("Delete", this);
  m_deleteButton->setToolTip("Delete the selected account and its profile data");
  connect(m_deleteButton, &QPushButton::clicked, this, &ProfileManagementDialog::onDelete);
  actionLayout->addWidget(m_deleteButton);

  mainLayout->addLayout(actionLayout);

  QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Close, this);
  buttonBox->button(QDialogButtonBox::Close)->setToolTip("Close this dialog");
  connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
  mainLayout->addWidget(buttonBox);

  connect(m_providerCombo, &QComboBox::currentIndexChanged, this, &ProfileManagementDialog::reload);

  reload();
}

void ProfileManagementDialog::reload() {
  m_profileList->clear();

  const QString providerId = m_providerCombo->currentData().toString();
  QList<Profile*> profiles = ProfileManager::instance()->profilesForProvider(providerId);

  for (Profile* profile : profiles) {
    QListWidgetItem* item = new QListWidgetItem(m_profileList);
    item->setText(QString("%1 (%2)").arg(profile->name(), profile->accountId()));
    item->setData(Qt::UserRole, profile->id());
    item->setData(Qt::UserRole + 1, profile->name());
    item->setToolTip(profile->path());
  }

  updateButtons();
}

void ProfileManagementDialog::onCreate() {
  const QString name = m_nameEdit->text().trimmed();
  if (name.isEmpty()) {
    return;
  }

  const QString providerId = m_providerCombo->currentData().toString();
  if (ProfileManager::instance()->createProfile(providerId, name)) {
    m_nameEdit->clear();
    reload();
  }
}

void ProfileManagementDialog::onRename() {
  QListWidgetItem* item = m_profileList->currentItem();
  const QString name = m_nameEdit->text().trimmed();
  if (!item || name.isEmpty()) {
    return;
  }

  Profile* profile = ProfileManager::instance()->profileById(item->data(Qt::UserRole).toString());
  if (ProfileManager::instance()->renameProfile(profile, name)) {
    reload();
  }
}

void ProfileManagementDialog::onDelete() {
  QListWidgetItem* item = m_profileList->currentItem();
  if (!item) {
    return;
  }

  Profile* profile = ProfileManager::instance()->profileById(item->data(Qt::UserRole).toString());
  if (!profile) {
    return;
  }

  const auto answer = QMessageBox::question(
      this, "Delete Account",
      QString("Delete the account \"%1\" and its browser data?").arg(profile->name()),
      QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
  if (answer != QMessageBox::Yes) {
    return;
  }

  if (!ProfileManager::instance()->deleteProfile(profile)) {
    QMessageBox::warning(this, "Delete Account",
                         "This account is currently in use by an open tab.");
    return;
  }

  reload();
}

void ProfileManagementDialog::updateButtons() {
  const bool hasSelection = m_profileList->currentItem() != nullptr;
  m_renameButton->setEnabled(hasSelection);
  m_deleteButton->setEnabled(hasSelection);
}
