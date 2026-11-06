#include "ProviderRegistry.h"
#include "Provider.h"
#include "ProviderIcons.h"

ProviderRegistry::ProviderRegistry(QObject* parent)
  : QObject(parent) {
  initializeProviders();
}

ProviderRegistry::~ProviderRegistry() {
  qDeleteAll(m_providers);
}

ProviderRegistry* ProviderRegistry::instance() {
  static ProviderRegistry instance;
  return &instance;
}

void ProviderRegistry::initializeProviders() {
  m_providers.clear();

  m_providers["deepseek"] = new Provider("deepseek", "DeepSeek", QUrl("https://chat.deepseek.com/"), this);
  m_providers["qwen"] = new Provider("qwen", "Qwen", QUrl("https://chat.qwen.ai/"), this);
  m_providers["glm"] = new Provider("glm", "GLM", QUrl("https://chat.z.ai/"), this);
  m_providers["copilot"] = new Provider("copilot", "Microsoft Copilot", QUrl("https://copilot.microsoft.com/"), this);
  m_providers["gemini"] = new Provider("gemini", "Google Gemini", QUrl("https://gemini.google.com/"), this);
  m_providers["claude"] = new Provider("claude", "Anthropic Claude", QUrl("https://claude.ai/"), this);
  m_providers["perplexity"] = new Provider("perplexity", "Perplexity", QUrl("https://www.perplexity.ai/"), this);
  m_providers["chatgpt"] = new Provider("chatgpt", "ChatGPT", QUrl("https://chatgpt.com/"), this);
  m_providers["kimi"] = new Provider("kimi", "Kimi", QUrl("https://kimi.ai/"), this);

  for (Provider* provider : m_providers) {
    provider->setIcon(providerIcon(provider->id()));
  }
}

QList<Provider*> ProviderRegistry::providers() const {
  return m_providers.values();
}

Provider* ProviderRegistry::providerById(const QString& id) const {
  return m_providers.value(id, nullptr);
}
