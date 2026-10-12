#include <QtTest/QtTest>
#include "ProviderRegistry.h"
#include "Provider.h"

class TestProviderRegistry : public QObject {
  Q_OBJECT
private slots:
  void testInstance();
  void testProvidersCount();
  void testProviderById();
  void testProviderIds();
};

void TestProviderRegistry::testInstance() {
  ProviderRegistry* instance1 = ProviderRegistry::instance();
  ProviderRegistry* instance2 = ProviderRegistry::instance();
  QCOMPARE(instance1, instance2);
}

void TestProviderRegistry::testProvidersCount() {
  ProviderRegistry* registry = ProviderRegistry::instance();
  QList<Provider*> providers = registry->providers();
  QCOMPARE(providers.size(), 9);
}

void TestProviderRegistry::testProviderById() {
  ProviderRegistry* registry = ProviderRegistry::instance();

  Provider* deepseek = registry->providerById("deepseek");
  QVERIFY(deepseek != nullptr);
  QCOMPARE(deepseek->id(), QString("deepseek"));
  QCOMPARE(deepseek->displayName(), QString("DeepSeek"));

  Provider* invalid = registry->providerById("invalid");
  QVERIFY(invalid == nullptr);
}

void TestProviderRegistry::testProviderIds() {
  ProviderRegistry* registry = ProviderRegistry::instance();
  QList<Provider*> providers = registry->providers();

  QStringList ids;
  for (Provider* provider : providers) {
    ids << provider->id();
  }

  QVERIFY(ids.contains("deepseek"));
  QVERIFY(ids.contains("qwen"));
  QVERIFY(ids.contains("glm"));
  QVERIFY(ids.contains("copilot"));
  QVERIFY(ids.contains("gemini"));
  QVERIFY(ids.contains("claude"));
  QVERIFY(ids.contains("perplexity"));
  QVERIFY(ids.contains("chatgpt"));
  QVERIFY(ids.contains("kimi"));
}

QTEST_MAIN(TestProviderRegistry)
#include "test_providerregistry.moc"
