#pragma once

#include <QDialog>
#include <QListWidget>

class Provider;
class QVBoxLayout;
class QHBoxLayout;
class QLabel;
class QLineEdit;
class QPushButton;

class ProviderSelectionDialog : public QDialog {
  Q_OBJECT
public:
  explicit ProviderSelectionDialog(QWidget* parent = nullptr);
  ~ProviderSelectionDialog() override = default;

signals:
  void providerSelected(const QString& providerId);

private slots:
  void onProviderSelected();
  void onFilterChanged(const QString& text);

private:
  void setupUI();
  void loadProviders();

  QListWidget* m_providerList;
  QLineEdit* m_filterEdit;
  QPushButton* m_createButton;
  QPushButton* m_cancelButton;
};
