#pragma once

#include "ThemedDialog.h"

class QComboBox;
class QLineEdit;
class QListWidget;
class QPushButton;

class ProfileManagementDialog : public ThemedDialog {
  Q_OBJECT
public:
  explicit ProfileManagementDialog(QWidget* parent = nullptr);
  ~ProfileManagementDialog() override = default;

private slots:
  void reload();
  void onCreate();
  void onRename();
  void onDelete();
  void updateButtons();

private:
  QComboBox* m_providerCombo;
  QListWidget* m_profileList;
  QLineEdit* m_nameEdit;
  QPushButton* m_renameButton;
  QPushButton* m_deleteButton;
};
