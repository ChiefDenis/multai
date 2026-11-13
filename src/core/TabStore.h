#pragma once

#include <QList>
#include <QStringList>
#include "TabMetadata.h"

class TabStore {
public:
  static constexpr int kSessionVersion = 1;

  static QList<TabMetadata> load();
  static void save(const QList<TabMetadata>& tabs, int currentIndex);
  static int currentIndex();
  static int savedVersion();
  static void clear();
};
