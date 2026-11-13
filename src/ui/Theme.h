#pragma once

#include <QPalette>
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
QPalette paletteForMode(Mode mode);
Mode effectiveMode(Mode mode);

}  // namespace Theme
