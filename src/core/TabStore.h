#pragma once

#include <QList>
#include "TabMetadata.h"

class TabStore {
public:
  static QList<TabMetadata> load();
  static void save(const QList<TabMetadata>& tabs, int currentIndex);
  static int currentIndex();
  static void clear();
};
