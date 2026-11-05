#include "Theme.h"

#include <QApplication>
#include <QGuiApplication>
#include <QPalette>

namespace Theme {

namespace {

const char* kLightStyleSheet = R"(
  QMainWindow, QDialog {
    background-color: #f6f6f6;
    color: #2e3436;
    font-size: 11pt;
  }
  QMenuBar {
    background-color: #ebebeb;
    color: #2e3436;
    border-bottom: 1px solid #dedede;
    padding: 2px 6px;
  }
  QMenuBar::item {
    background: transparent;
    padding: 6px 10px;
    border-radius: 6px;
  }
  QMenuBar::item:selected {
    background-color: #dedede;
  }
  QMenu {
    background-color: #ffffff;
    color: #2e3436;
    border: 1px solid #d5d5d5;
    border-radius: 8px;
    padding: 4px;
  }
  QMenu::item {
    padding: 6px 24px 6px 12px;
    border-radius: 6px;
  }
  QMenu::item:selected {
    background-color: #3584e4;
    color: #ffffff;
  }
  QMenu::separator {
    height: 1px;
    background-color: #e5e5e5;
    margin: 4px 6px;
  }
  QToolBar {
    background-color: #f6f6f6;
    border-bottom: 1px solid #dedede;
    spacing: 8px;
    padding: 6px 8px;
  }
  QToolButton {
    background-color: #ffffff;
    border: 1px solid #dcdcdc;
    border-radius: 6px;
    padding: 5px 12px;
    color: #2e3436;
    font-weight: 500;
  }
  QToolButton:hover {
    background-color: #f0f0f0;
    border-color: #cecece;
  }
  QToolButton:pressed {
    background-color: #e5e5e5;
  }
  QTabWidget::pane {
    border: none;
    background-color: #ffffff;
  }
  QTabBar {
    background-color: #f0f0f0;
    border-bottom: 1px solid #dedede;
  }
  QTabBar::tab {
    background-color: #e6e6e6;
    color: #5c616c;
    border: 1px solid #dedede;
    border-bottom: none;
    border-top-left-radius: 8px;
    border-top-right-radius: 8px;
    padding: 6px 16px;
    margin-right: 3px;
    margin-top: 4px;
    font-weight: 500;
  }
  QTabBar::tab:selected {
    background-color: #ffffff;
    color: #1c71d8;
    border-color: #dedede;
    font-weight: 600;
  }
  QTabBar::tab:hover:!selected {
    background-color: #eeeeee;
    color: #2e3436;
  }
  QTabBar::close-button {
    subcontrol-position: right;
    border-radius: 6px;
    padding: 2px;
  }
  QTabBar::close-button:hover {
    background-color: #e01b24;
    color: #ffffff;
  }
  QPushButton {
    background-color: #ffffff;
    color: #2e3436;
    border: 1px solid #dcdcdc;
    border-radius: 6px;
    padding: 6px 16px;
    font-weight: 500;
  }
  QPushButton:hover {
    background-color: #f5f5f5;
    border-color: #cecece;
  }
  QPushButton:pressed {
    background-color: #e8e8e8;
  }
  QPushButton:disabled {
    background-color: #f0f0f0;
    color: #9a9996;
    border-color: #e4e4e4;
  }
  QPushButton[default="true"] {
    background-color: #3584e4;
    color: #ffffff;
    border: none;
  }
  QPushButton[default="true"]:hover {
    background-color: #1c71d8;
  }
  QPushButton[default="true"]:pressed {
    background-color: #1a5fb4;
  }
  QLineEdit, QComboBox, QSpinBox {
    background-color: #ffffff;
    color: #2e3436;
    border: 1px solid #dcdcdc;
    border-radius: 6px;
    padding: 6px 10px;
    selection-background-color: #3584e4;
    selection-color: #ffffff;
  }
  QLineEdit:focus, QComboBox:focus, QSpinBox:focus {
    border: 2px solid #3584e4;
    padding: 5px 9px;
  }
  QListWidget {
    background-color: #ffffff;
    color: #2e3436;
    border: 1px solid #dcdcdc;
    border-radius: 8px;
    padding: 4px;
  }
  QListWidget::item {
    padding: 8px 10px;
    border-radius: 6px;
  }
  QListWidget::item:selected {
    background-color: #3584e4;
    color: #ffffff;
  }
  QListWidget::item:hover:!selected {
    background-color: #f5f5f5;
  }
  QStatusBar {
    background-color: #f6f6f6;
    color: #5c616c;
    border-top: 1px solid #dedede;
    padding: 3px 8px;
  }
  QScrollBar:vertical {
    background: transparent;
    width: 8px;
    margin: 0;
  }
  QScrollBar::handle:vertical {
    background: #c0c0c0;
    border-radius: 4px;
    min-height: 24px;
  }
  QScrollBar::handle:vertical:hover {
    background: #9a9996;
  }
  QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical,
  QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical {
    background: none;
    height: 0;
  }
)";

const char* kDarkStyleSheet = R"(
  QMainWindow, QDialog {
    background-color: #242424;
    color: #ffffff;
    font-size: 11pt;
  }
  QMenuBar {
    background-color: #2e2e2e;
    color: #ffffff;
    border-bottom: 1px solid #383838;
    padding: 2px 6px;
  }
  QMenuBar::item {
    background: transparent;
    padding: 6px 10px;
    border-radius: 6px;
  }
  QMenuBar::item:selected {
    background-color: #3d3d3d;
  }
  QMenu {
    background-color: #2e2e2e;
    color: #ffffff;
    border: 1px solid #3d3d3d;
    border-radius: 8px;
    padding: 4px;
  }
  QMenu::item {
    padding: 6px 24px 6px 12px;
    border-radius: 6px;
  }
  QMenu::item:selected {
    background-color: #3584e4;
    color: #ffffff;
  }
  QMenu::separator {
    height: 1px;
    background-color: #3d3d3d;
    margin: 4px 6px;
  }
  QToolBar {
    background-color: #242424;
    border-bottom: 1px solid #383838;
    spacing: 8px;
    padding: 6px 8px;
  }
  QToolButton {
    background-color: #303030;
    border: 1px solid #3d3d3d;
    border-radius: 6px;
    padding: 5px 12px;
    color: #ffffff;
    font-weight: 500;
  }
  QToolButton:hover {
    background-color: #3d3d3d;
    border-color: #4a4a4a;
  }
  QToolButton:pressed {
    background-color: #282828;
  }
  QTabWidget::pane {
    border: none;
    background-color: #1e1e1e;
  }
  QTabBar {
    background-color: #1a1a1a;
    border-bottom: 1px solid #383838;
  }
  QTabBar::tab {
    background-color: #282828;
    color: #9a9996;
    border: 1px solid #383838;
    border-bottom: none;
    border-top-left-radius: 8px;
    border-top-right-radius: 8px;
    padding: 6px 16px;
    margin-right: 3px;
    margin-top: 4px;
    font-weight: 500;
  }
  QTabBar::tab:selected {
    background-color: #1e1e1e;
    color: #78aeed;
    border-color: #383838;
    font-weight: 600;
  }
  QTabBar::tab:hover:!selected {
    background-color: #303030;
    color: #ffffff;
  }
  QTabBar::close-button {
    subcontrol-position: right;
    border-radius: 6px;
    padding: 2px;
  }
  QTabBar::close-button:hover {
    background-color: #e01b24;
    color: #ffffff;
  }
  QPushButton {
    background-color: #303030;
    color: #ffffff;
    border: 1px solid #3d3d3d;
    border-radius: 6px;
    padding: 6px 16px;
    font-weight: 500;
  }
  QPushButton:hover {
    background-color: #3d3d3d;
    border-color: #4a4a4a;
  }
  QPushButton:pressed {
    background-color: #262626;
  }
  QPushButton:disabled {
    background-color: #242424;
    color: #5e5e5e;
    border-color: #2f2f2f;
  }
  QPushButton[default="true"] {
    background-color: #3584e4;
    color: #ffffff;
    border: none;
  }
  QPushButton[default="true"]:hover {
    background-color: #1c71d8;
  }
  QPushButton[default="true"]:pressed {
    background-color: #1a5fb4;
  }
  QLineEdit, QComboBox, QSpinBox {
    background-color: #2a2a2a;
    color: #ffffff;
    border: 1px solid #3d3d3d;
    border-radius: 6px;
    padding: 6px 10px;
    selection-background-color: #3584e4;
    selection-color: #ffffff;
  }
  QLineEdit:focus, QComboBox:focus, QSpinBox:focus {
    border: 2px solid #78aeed;
    padding: 5px 9px;
  }
  QListWidget {
    background-color: #242424;
    color: #ffffff;
    border: 1px solid #383838;
    border-radius: 8px;
    padding: 4px;
  }
  QListWidget::item {
    padding: 8px 10px;
    border-radius: 6px;
  }
  QListWidget::item:selected {
    background-color: #3584e4;
    color: #ffffff;
  }
  QListWidget::item:hover:!selected {
    background-color: #2e2e2e;
  }
  QStatusBar {
    background-color: #242424;
    color: #9a9996;
    border-top: 1px solid #383838;
    padding: 3px 8px;
  }
  QScrollBar:vertical {
    background: transparent;
    width: 8px;
    margin: 0;
  }
  QScrollBar::handle:vertical {
    background: #484848;
    border-radius: 4px;
    min-height: 24px;
  }
  QScrollBar::handle:vertical:hover {
    background: #606060;
  }
  QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical,
  QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical {
    background: none;
    height: 0;
  }
)";

}  // namespace

Mode modeFromString(const QString& name) {
  if (name == "dark") {
    return Mode::Dark;
  }
  if (name == "light") {
    return Mode::Light;
  }
  return Mode::System;
}

QString modeToString(Mode mode) {
  switch (mode) {
    case Mode::Dark:
      return "dark";
    case Mode::Light:
      return "light";
    case Mode::System:
      return "system";
  }
  return "system";
}

QString styleSheetForMode(Mode mode) {
  if (mode == Mode::Dark) {
    return QLatin1String(kDarkStyleSheet);
  }
  if (mode == Mode::Light) {
    return QLatin1String(kLightStyleSheet);
  }

  const bool isDark = QGuiApplication::palette().window().color().lightness() < 128;
  return isDark ? QLatin1String(kDarkStyleSheet) : QLatin1String(kLightStyleSheet);
}

void applyTheme(const QString& themeName) {
  if (qApp) {
    qApp->setStyleSheet(styleSheetForMode(modeFromString(themeName)));
  }
}

}  // namespace Theme
