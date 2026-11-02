#include "ProviderSelectionDialog.h"
#include "providers/ProviderRegistry.h"
#include "providers/Provider.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QListWidgetItem>
#include <QDialogButtonBox>

ProviderSelectionDialog::ProviderSelectionDialog(QWidget* parent)
  : QDialog(parent)
  , m_providerList(nullptr)
  , m_filterEdit(nullptr)
  , m_createButton(nullptr)
  , m_cancelButton(nullptr) {
  setWindowTitle("Select Provider");
  setMinimumSize(500, 400);

  setupUI();
  loadProviders();
}

void ProviderSelectionDialog::setupUI() {
  QVBoxLayout* mainLayout = new QVBoxLayout(this);

  QLabel* titleLabel = new QLabel("Select a provider", this);
  QFont titleFont = titleLabel->font();
  titleFont.setPointSize(14);
  titleFont.setBold(true);
  titleLabel->setFont(titleFont);
  mainLayout->addWidget(titleLabel);

  m_filterEdit = new QLineEdit(this);
  m_filterEdit->setPlaceholderText("Search providers...");
  m_filterEdit->setToolTip("Filter providers by name");
  connect(m_filterEdit, &QLineEdit::textChanged, this, &ProviderSelectionDialog::onFilterChanged);
  mainLayout->addWidget(m_filterEdit);

  m_providerList = new QListWidget(this);
  m_providerList->setAlternatingRowColors(true);
  m_providerList->setSelectionMode(QAbstractItemView::SingleSelection);
  m_providerList->setToolTip("Select a provider from the list");
  connect(m_providerList, &QListWidget::itemDoubleClicked, this, &ProviderSelectionDialog::onProviderSelected);
  mainLayout->addWidget(m_providerList);

  QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
  m_createButton = buttonBox->button(QDialogButtonBox::Ok);
  m_createButton->setText("Create Tab");
  m_createButton->setEnabled(false);
  m_createButton->setToolTip("Create a new tab with the selected provider");
  connect(m_createButton, &QPushButton::clicked, this, &ProviderSelectionDialog::onProviderSelected);

  m_cancelButton = buttonBox->button(QDialogButtonBox::Cancel);
  m_cancelButton->setToolTip("Cancel and close dialog");
  connect(m_cancelButton, &QPushButton::clicked, this, &QDialog::reject);

  connect(m_providerList, &QListWidget::itemSelectionChanged, [this]() {
    m_createButton->setEnabled(m_providerList->currentItem() != nullptr);
  });

  mainLayout->addWidget(buttonBox);
}

void ProviderSelectionDialog::loadProviders() {
  ProviderRegistry* registry = ProviderRegistry::instance();
  QList<Provider*> providers = registry->providers();

  for (Provider* provider : providers) {
    QListWidgetItem* item = new QListWidgetItem(m_providerList);
    item->setText(provider->displayName());
    item->setData(Qt::UserRole, provider->id());
    item->setToolTip(QString("Open %1").arg(provider->displayName()));
  }
}

void ProviderSelectionDialog::onProviderSelected() {
  QListWidgetItem* currentItem = m_providerList->currentItem();
  if (currentItem) {
    QString providerId = currentItem->data(Qt::UserRole).toString();
    emit providerSelected(providerId);
    accept();
  }
}

void ProviderSelectionDialog::onFilterChanged(const QString& text) {
  for (int i = 0; i < m_providerList->count(); ++i) {
    QListWidgetItem* item = m_providerList->item(i);
    bool matches = text.isEmpty() || item->text().contains(text, Qt::CaseInsensitive);
    item->setHidden(!matches);
  }
}
