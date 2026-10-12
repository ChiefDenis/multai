#pragma once

#include <QObject>
#include <QList>
#include <QMap>

class Provider;

class ProviderRegistry : public QObject {
  Q_OBJECT
public:
  static ProviderRegistry* instance();

  QList<Provider*> providers() const;
  Provider* providerById(const QString& id) const;

private:
  explicit ProviderRegistry(QObject* parent = nullptr);
  ~ProviderRegistry() override;

  void initializeProviders();

  QMap<QString, Provider*> m_providers;
};
