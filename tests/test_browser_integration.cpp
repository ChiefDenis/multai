#include <QtTest/QtTest>
#include <QSignalSpy>
#include <QStandardPaths>
#include <QUrl>

#include "lifecycle/BrowserTab.h"
#include "lifecycle/BrowserView.h"
#include "providers/ProviderRegistry.h"
#include "providers/Provider.h"

class TestBrowserIntegration : public QObject {
  Q_OBJECT
private slots:
  void initTestCase();
  void testAllNineProvidersRegistered();
  void testProviderURLsValid();
  void testBrowserTabPageCreation();
  void testBrowserTabLoadSuccess();
  void testBrowserTabLoadFailure();
  void testTabTitleUpdate();
  void testErrorRecovery();
  void testBrowserViewCreation();
};

void TestBrowserIntegration::initTestCase() {
  qputenv("QT_QPA_PLATFORM", "offscreen");
  QStandardPaths::setTestModeEnabled(true);
}

void TestBrowserIntegration::testAllNineProvidersRegistered() {
  ProviderRegistry* registry = ProviderRegistry::instance();
  QList<Provider*> providers = registry->providers();
  QCOMPARE(providers.size(), 9);

  QStringList expectedIds = {
      "deepseek", "qwen", "glm", "copilot", "gemini",
      "claude", "perplexity", "chatgpt", "kimi"};
  for (Provider* p : providers) {
    QVERIFY(expectedIds.contains(p->id()));
    QVERIFY(!p->displayName().isEmpty());
    QVERIFY(p->url().isValid());
    QVERIFY(!p->url().isEmpty());
  }
}

void TestBrowserIntegration::testProviderURLsValid() {
  ProviderRegistry* registry = ProviderRegistry::instance();

  QCOMPARE(registry->providerById("deepseek")->url(),
           QUrl("https://www.deepseek.com/"));
  QCOMPARE(registry->providerById("qwen")->url(), QUrl("https://qwen.ai/home"));
  QCOMPARE(registry->providerById("glm")->url(), QUrl("https://z.ai"));
  QCOMPARE(registry->providerById("copilot")->url(),
           QUrl("https://copilot.microsoft.com/"));
  QCOMPARE(registry->providerById("gemini")->url(),
           QUrl("https://gemini.google.com/"));
  QCOMPARE(registry->providerById("claude")->url(), QUrl("https://claude.ai/"));
  QCOMPARE(registry->providerById("perplexity")->url(),
           QUrl("https://www.perplexity.ai/"));
  QCOMPARE(registry->providerById("chatgpt")->url(), QUrl("https://chatgpt.com/"));
  QCOMPARE(registry->providerById("kimi")->url(), QUrl("https://www.kimi.ai/"));
}

void TestBrowserIntegration::testBrowserTabPageCreation() {
  BrowserTab tab("deepseek", "default", QUrl("https://www.deepseek.com/"),
                 LifecycleState::Unloaded);

  // Initially no page
  QVERIFY(tab.page() == nullptr);
  QVERIFY(tab.view() == nullptr);
  QVERIFY(!tab.pageCreated());

  // Activate creates page
  tab.activate();
  QCOMPARE(tab.lifecycleState(), LifecycleState::Loading);
  QVERIFY(tab.pageCreated());
  QVERIFY(tab.page() != nullptr);
  QVERIFY(tab.view() != nullptr);

  tab.closeTab();
}

void TestBrowserIntegration::testBrowserTabLoadSuccess() {
  BrowserTab tab("deepseek", "default", QUrl("https://www.deepseek.com/"));

  tab.activate();
  QCOMPARE(tab.lifecycleState(), LifecycleState::Loading);

  QSignalSpy loadSpy(&tab, &BrowserTab::loadFinished);
  tab.onLoadFinished(true);

  QCOMPARE(tab.lifecycleState(), LifecycleState::Active);
  QCOMPARE(tab.hasError(), false);
  QCOMPARE(loadSpy.count(), 1);
  QCOMPARE(loadSpy.takeFirst().at(0).toBool(), true);

  tab.closeTab();
}

void TestBrowserIntegration::testBrowserTabLoadFailure() {
  BrowserTab tab("deepseek", "default", QUrl("https://www.deepseek.com/"));

  tab.activate();
  QCOMPARE(tab.lifecycleState(), LifecycleState::Loading);

  QSignalSpy errorSpy(&tab, &BrowserTab::errorChanged);
  tab.onLoadFinished(false);

  QCOMPARE(tab.lifecycleState(), LifecycleState::Unloaded);
  QCOMPARE(tab.hasError(), true);
  QCOMPARE(tab.errorTitle(), QString("Unable to Load Page"));
  QVERIFY(tab.errorMessage().contains("deepseek.com"));
  QCOMPARE(errorSpy.count(), 1);
  QCOMPARE(errorSpy.takeFirst().at(0).toBool(), true);

  tab.closeTab();
}

void TestBrowserIntegration::testTabTitleUpdate() {
  BrowserTab tab("deepseek", "default", QUrl("https://www.deepseek.com/"));

  QSignalSpy titleSpy(&tab, &BrowserTab::titleChanged);
  tab.onTitleChanged("DeepSeek - AI Assistant");

  QCOMPARE(tab.title(), QString("DeepSeek - AI Assistant"));
  QCOMPARE(titleSpy.count(), 1);

  tab.closeTab();
}

void TestBrowserIntegration::testErrorRecovery() {
  BrowserTab tab("deepseek", "default", QUrl("https://www.deepseek.com/"));

  // Cause load failure
  tab.activate();
  tab.onLoadFinished(false);
  QCOMPARE(tab.hasError(), true);

  // Retry should clear error and reload
  QSignalSpy errorSpy(&tab, &BrowserTab::errorChanged);
  tab.retry();

  QCOMPARE(tab.hasError(), false);
  QCOMPARE(tab.lifecycleState(), LifecycleState::Loading);
  QCOMPARE(errorSpy.count(), 1);
  QCOMPARE(errorSpy.takeFirst().at(0).toBool(), false);

  tab.closeTab();
}

void TestBrowserIntegration::testBrowserViewCreation() {
  BrowserView view;
  // QWebEngineView always has a default page
  QVERIFY(view.page() != nullptr);
  QVERIFY(view.inherits("QWebEngineView"));
}

QTEST_MAIN(TestBrowserIntegration)
#include "test_browser_integration.moc"
