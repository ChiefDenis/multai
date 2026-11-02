#pragma once

#include <QDialog>

class QComboBox;
class QLineEdit;
class QPushButton;

class ProfileSelectionDialog : public QDialog {
  Q_OBJECT
public:
  ProfileSelectionDialog(const QString& providerId, const QString& providerName,
                         QWidget* parent = nullptr);
  ~ProfileSelectionDialog() override = default;

  QString selectedAccountId() const;

private slots:
  void onCreateAccount();
  void updateOkState();

private:
  void reloadAccounts();

  QString m_providerId;
  QComboBox* m_accountCombo;
  QLineEdit* m_newAccountEdit;
  QPushButton* m_okButton;
};
