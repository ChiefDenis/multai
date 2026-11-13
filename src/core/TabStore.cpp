#include "TabStore.h"
#include "SettingsManager.h"
#include "providers/Provider.h"
#include "providers/ProviderRegistry.h"

#include <QVariantList>
#include <QVariantMap>

namespace {

const char* const kTabsKey = "workspace/openTabs";
const char* const kCurrentKey = "workspace/currentTabIndex";
const char* const kVersionKey = "workspace/sessionVersion";
constexpr int kMaxSessionTabs = 50;

}  // namespace

QList<TabMetadata> TabStore::load() {
  QList<TabMetadata> tabs;
  SettingsManager* settings = SettingsManager::instance();

  const int version = settings->value(kVersionKey, kSessionVersion).toInt();
  if (version < 1 || version > kSessionVersion) {
    return tabs;
  }

  ProviderRegistry* registry = ProviderRegistry::instance();
  const QVariantList stored = settings->value(kTabsKey).toList();
  for (const QVariant& entry : stored) {
    if (tabs.size() >= kMaxSessionTabs) {
      break;
    }

    const QVariantMap map = entry.toMap();
    const QString providerId = map.value("providerId").toString();
    Provider* provider = registry->providerById(providerId);
    if (!provider) {
      continue;
    }

    const QUrl savedUrl(map.value("url").toString());
    const QUrl url = provider->isUrlAllowed(savedUrl) ? savedUrl : provider->url();

    TabMetadata metadata(providerId, map.value("profileId").toString(), url);
    metadata.setTitle(map.value("title").toString().trimmed());
    const QString stateStr = map.value("lifecycleState").toString();
    metadata.setLifecycleState(lifecycleStateFromString(stateStr));
    tabs.append(metadata);
  }

  return tabs;
}

void TabStore::save(const QList<TabMetadata>& tabs, int currentIndex) {
  QVariantList stored;
  for (const TabMetadata& metadata : tabs) {
    QVariantMap map;
    map["providerId"] = metadata.providerId();
    map["profileId"] = metadata.profileId();
    map["url"] = metadata.url().toString();
    map["title"] = metadata.title();
    map["lifecycleState"] = lifecycleStateToString(metadata.lifecycleState());
    stored.append(map);
  }

  SettingsManager* settings = SettingsManager::instance();
  settings->setValue(kVersionKey, kSessionVersion);
  settings->setValue(kTabsKey, stored);
  settings->setValue(kCurrentKey, currentIndex);
}

int TabStore::currentIndex() {
  return SettingsManager::instance()->value(kCurrentKey, 0).toInt();
}

int TabStore::savedVersion() {
  return SettingsManager::instance()->value(kVersionKey, 0).toInt();
}

void TabStore::clear() {
  SettingsManager* settings = SettingsManager::instance();
  settings->reset(kTabsKey);
  settings->reset(kCurrentKey);
  settings->reset(kVersionKey);
}
