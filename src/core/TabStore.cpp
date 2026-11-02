#include "TabStore.h"
#include "SettingsManager.h"

#include <QVariantList>
#include <QVariantMap>

namespace {

const char* const kTabsKey = "workspace/openTabs";
const char* const kCurrentKey = "workspace/currentTabIndex";

}  // namespace

QList<TabMetadata> TabStore::load() {
  QList<TabMetadata> tabs;

  const QVariantList stored = SettingsManager::instance()->value(kTabsKey).toList();
  for (const QVariant& entry : stored) {
    const QVariantMap map = entry.toMap();

    const QString providerId = map.value("providerId").toString();
    const QUrl url(map.value("url").toString());
    if (providerId.isEmpty() || !url.isValid() || url.isEmpty()) {
      continue;
    }

    TabMetadata metadata(providerId, map.value("profileId").toString(), url);
    metadata.setTitle(map.value("title").toString());
    metadata.setLifecycleState(LifecycleState::Unloaded);
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
    stored.append(map);
  }

  SettingsManager* settings = SettingsManager::instance();
  settings->setValue(kTabsKey, stored);
  settings->setValue(kCurrentKey, currentIndex);
}

int TabStore::currentIndex() {
  return SettingsManager::instance()->value(kCurrentKey, 0).toInt();
}

void TabStore::clear() {
  SettingsManager* settings = SettingsManager::instance();
  settings->reset(kTabsKey);
  settings->reset(kCurrentKey);
}
