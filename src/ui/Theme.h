#pragma once

#include <QString>

namespace Theme {

enum class Mode {
  System,
  Light,
  Dark
};

Mode modeFromString(const QString& name);
QString modeToString(Mode mode);
QString styleSheetForMode(Mode mode);
void applyTheme(const QString& themeName);

}  // namespace Theme
