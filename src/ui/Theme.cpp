#include "Theme.h"

#include <QApplication>
#include <QGuiApplication>
#include <QPalette>

namespace Theme {

namespace {

// Light theme semantic colors
const char* kLightStyleSheet = R"(
  /* Color palette (light) */
  * {
    color-scheme: light;
  }
  QMainWindow, QDialog {
    background-color: #f8f9fa;
    color: #212529;
    font-size: 11pt;
    selection-background-color: #3584e4;
    selection-color: #ffffff;
  }
  /* Application frame */
  #appFrame {
    background-color: #f8f9fa;
    border: none;
  }
  /* Menu bar */
  QMenuBar {
    background-color: #ffffff;
    color: #212529;
    border: none;
    border-bottom: 1px solid #dee2e6;
    padding: 4px 8px;
  }
  QMenuBar::item {
    background: transparent;
    padding: 6px 12px;
    border-radius: 6px;
    color: #212529;
  }
  QMenuBar::item:selected {
    background-color: #e9ecef;
    color: #212529;
  }
  QMenu {
    background-color: #ffffff;
    color: #212529;
    border: 1px solid #dee2e6;
    border-radius: 8px;
    padding: 6px;
  }
  QMenu::item {
    padding: 7px 28px 7px 14px;
    border-radius: 6px;
    color: #212529;
  }
  QMenu::item:selected {
    background-color: #3584e4;
    color: #ffffff;
  }
  QMenu::separator {
    height: 1px;
    background-color: #e9ecef;
    margin: 6px 8px;
  }
  /* Sidebar */
  #sidebar {
    background-color: #ffffff;
    border-right: 1px solid #dee2e6;
  }
  #sidebarHeader {
    background-color: #ffffff;
    border-bottom: 1px solid #dee2e6;
  }
  #sidebarHeader QLabel {
    color: #212529;
    background: transparent;
    border: none;
  }
  #sidebarSectionLabel {
    color: #6c757d;
    background: transparent;
    border: none;
  }
  #providerList, #tabList {
    background-color: #ffffff;
    border: none;
    border-radius: 0;
    outline: none;
    padding: 4px;
  }
  #providerList::item, #tabList::item {
    border: none;
    padding: 0;
    margin: 2px 4px;
    background: transparent;
  }
  #providerList QWidget, #tabList QWidget {
    background: transparent;
  }
  #providerNameLabel {
    color: #212529;
    background: transparent;
    border: none;
  }
  #providerIconLabel {
    background: transparent;
    border: none;
  }
  #tabRowWidget {
    background-color: #f8f9fa;
    border: 1px solid #dee2e6;
    border-radius: 8px;
  }
  #tabRowWidget:hover {
    background-color: #e9ecef;
    border-color: #adb5bd;
  }
  #tabRowWidget:selected {
    background-color: #e3f2fd;
    border-color: #3584e4;
  }
  #tabTitleLabel {
    color: #212529;
    background: transparent;
    border: none;
  }
  #tabIconLabel {
    background: transparent;
    border: none;
  }
  #tabStateBadge {
    background-color: #6c757d;
    color: #ffffff;
    border-radius: 8px;
    font-weight: bold;
  }
  #tabCloseButton {
    background: transparent;
    border: none;
    border-radius: 6px;
    padding: 4px;
  }
  #tabCloseButton:hover {
    background-color: #e01b24;
  }
  #tabCloseButton:pressed {
    background-color: #c4121a;
  }
  #sidebarBottomSection {
    background-color: #f8f9fa;
    border-top: 1px solid #dee2e6;
  }
  #sidebarCollapseButton, #sidebarSettingsButton {
    background: transparent;
    border: none;
    border-radius: 6px;
    padding: 6px;
  }
  #sidebarCollapseButton:hover, #sidebarSettingsButton:hover {
    background-color: #e9ecef;
  }
  #sidebarCollapseButton:pressed, #sidebarSettingsButton:pressed {
    background-color: #dee2e6;
  }
  /* Workspace header */
  #workspaceHeader {
    background-color: #ffffff;
    border-bottom: 1px solid #dee2e6;
  }
  #workspaceProviderLabel {
    color: #212529;
    background: transparent;
    border: none;
  }
  #workspaceProviderIcon {
    background: transparent;
    border: none;
  }
  /* Empty state */
  #emptyState {
    background-color: #f8f9fa;
    color: #212529;
  }
  #emptyState QLabel {
    background: transparent;
    border: none;
  }
  #emptyStateTitle {
    color: #212529;
    font-size: 20pt;
    font-weight: 600;
  }
  #emptyStateSubtitle {
    color: #6c757d;
    font-size: 11pt;
  }
  #emptyStateProviderFrame {
    background-color: #ffffff;
    border: 1px solid #dee2e6;
    border-radius: 10px;
  }
  #emptyStateProviderButton {
    background-color: #ffffff;
    color: #212529;
    border: 1px solid #dee2e6;
    border-radius: 8px;
    padding: 10px;
    text-align: center;
  }
  #emptyStateProviderButton:hover {
    background-color: #e9ecef;
    border-color: #adb5bd;
    color: #212529;
  }
  #emptyStateProviderButton:pressed {
    background-color: #dee2e6;
    border-color: #adb5bd;
  }
  #emptyStateProviderButton:focus {
    border: 2px solid #3584e4;
    outline: none;
  }
  /* Generic controls */
  QToolButton {
    background-color: #ffffff;
    color: #212529;
    border: 1px solid #dee2e6;
    border-radius: 6px;
    padding: 6px 14px;
    font-weight: 500;
  }
  QToolButton:hover {
    background-color: #f8f9fa;
    border-color: #adb5bd;
  }
  QToolButton:pressed {
    background-color: #e9ecef;
  }
  QToolButton:disabled {
    background-color: #f8f9fa;
    color: #adb5bd;
    border-color: #e9ecef;
  }
  QToolButton:focus {
    border: 2px solid #3584e4;
    padding: 5px 13px;
  }
  QPushButton {
    background-color: #ffffff;
    color: #212529;
    border: 1px solid #dee2e6;
    border-radius: 6px;
    padding: 6px 16px;
    font-weight: 500;
  }
  QPushButton:hover {
    background-color: #f8f9fa;
    border-color: #adb5bd;
  }
  QPushButton:pressed {
    background-color: #e9ecef;
  }
  QPushButton:disabled {
    background-color: #f8f9fa;
    color: #adb5bd;
    border-color: #e9ecef;
  }
  QPushButton:focus {
    border: 2px solid #3584e4;
    padding: 5px 15px;
  }
  QPushButton[destructive="true"] {
    background-color: #fff5f5;
    color: #e01b24;
    border: 1px solid #ffcccc;
  }
  QPushButton[destructive="true"]:hover {
    background-color: #fdecea;
    border-color: #e01b24;
  }
  QPushButton[destructive="true"]:pressed {
    background-color: #fcd9d9;
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
    color: #212529;
    border: 1px solid #dee2e6;
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
    color: #212529;
    border: 1px solid #dee2e6;
    border-radius: 8px;
    padding: 4px;
    outline: none;
  }
  QListWidget::item {
    padding: 8px 12px;
    border-radius: 6px;
    border: none;
    margin: 1px 2px;
  }
  QListWidget::item:selected {
    background-color: #e3f2fd;
    color: #212529;
    border: 1px solid #3584e4;
  }
  QListWidget::item:hover:!selected {
    background-color: #f8f9fa;
  }
  QListWidget::item:focus {
    border: 2px solid #3584e4;
  }
  QStatusBar {
    background-color: #f8f9fa;
    color: #6c757d;
    border: none;
    border-top: 1px solid #dee2e6;
    padding: 4px 8px;
  }
  QScrollBar:vertical {
    background: transparent;
    width: 10px;
    margin: 0;
    border: none;
  }
  QScrollBar::handle:vertical {
    background: #adb5bd;
    border-radius: 5px;
    min-height: 30px;
    margin: 2px;
  }
  QScrollBar::handle:vertical:hover {
    background: #868e96;
  }
  QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical,
  QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical {
    background: none;
    height: 0;
  }
  QScrollBar:horizontal {
    background: transparent;
    height: 10px;
    margin: 0;
    border: none;
  }
  QScrollBar::handle:horizontal {
    background: #adb5bd;
    border-radius: 5px;
    min-width: 30px;
    margin: 2px;
  }
  QScrollBar::handle:horizontal:hover {
    background: #868e96;
  }
  QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal,
  QScrollBar::add-page:horizontal, QScrollBar::sub-page:horizontal {
    background: none;
    width: 0;
  }
)";

// Dark theme semantic colors
const char* kDarkStyleSheet = R"(
  /* Color palette (dark) */
  * {
    color-scheme: dark;
  }
  QMainWindow, QDialog {
    background-color: #161616;
    color: #e0e0e0;
    font-size: 11pt;
    selection-background-color: #78aeed;
    selection-color: #161616;
  }
  /* Application frame */
  #appFrame {
    background-color: #161616;
    border: none;
  }
  /* Menu bar */
  QMenuBar {
    background-color: #1e1e1e;
    color: #e0e0e0;
    border: none;
    border-bottom: 1px solid #333333;
    padding: 4px 8px;
  }
  QMenuBar::item {
    background: transparent;
    padding: 6px 12px;
    border-radius: 6px;
    color: #e0e0e0;
  }
  QMenuBar::item:selected {
    background-color: #2a2a2a;
    color: #ffffff;
  }
  QMenu {
    background-color: #1e1e1e;
    color: #e0e0e0;
    border: 1px solid #333333;
    border-radius: 8px;
    padding: 6px;
  }
  QMenu::item {
    padding: 7px 28px 7px 14px;
    border-radius: 6px;
    color: #e0e0e0;
  }
  QMenu::item:selected {
    background-color: #78aeed;
    color: #161616;
  }
  QMenu::separator {
    height: 1px;
    background-color: #2a2a2a;
    margin: 6px 8px;
  }
  /* Sidebar */
  #sidebar {
    background-color: #1e1e1e;
    border-right: 1px solid #333333;
  }
  #sidebarHeader {
    background-color: #1e1e1e;
    border-bottom: 1px solid #333333;
  }
  #sidebarHeader QLabel {
    color: #e0e0e0;
    background: transparent;
    border: none;
  }
  #sidebarSectionLabel {
    color: #a0a0a0;
    background: transparent;
    border: none;
  }
  #providerList, #tabList {
    background-color: #1e1e1e;
    border: none;
    border-radius: 0;
    outline: none;
    padding: 4px;
  }
  #providerList::item, #tabList::item {
    border: none;
    padding: 0;
    margin: 2px 4px;
    background: transparent;
  }
  #providerList QWidget, #tabList QWidget {
    background: transparent;
  }
  #providerNameLabel {
    color: #e0e0e0;
    background: transparent;
    border: none;
  }
  #providerIconLabel {
    background: transparent;
    border: none;
  }
  #tabRowWidget {
    background-color: #242424;
    border: 1px solid #333333;
    border-radius: 8px;
  }
  #tabRowWidget:hover {
    background-color: #2a2a2a;
    border-color: #4a4a4a;
  }
  #tabRowWidget:selected {
    background-color: #2a3b5c;
    border-color: #78aeed;
  }
  #tabTitleLabel {
    color: #e0e0e0;
    background: transparent;
    border: none;
  }
  #tabIconLabel {
    background: transparent;
    border: none;
  }
  #tabStateBadge {
    background-color: #6c757d;
    color: #ffffff;
    border-radius: 8px;
    font-weight: bold;
  }
  #tabCloseButton {
    background: transparent;
    border: none;
    border-radius: 6px;
    padding: 4px;
  }
  #tabCloseButton:hover {
    background-color: #e01b24;
  }
  #tabCloseButton:pressed {
    background-color: #c4121a;
  }
  #sidebarBottomSection {
    background-color: #1e1e1e;
    border-top: 1px solid #333333;
  }
  #sidebarCollapseButton, #sidebarSettingsButton {
    background: transparent;
    border: none;
    border-radius: 6px;
    padding: 6px;
  }
  #sidebarCollapseButton:hover, #sidebarSettingsButton:hover {
    background-color: #2a2a2a;
  }
  #sidebarCollapseButton:pressed, #sidebarSettingsButton:pressed {
    background-color: #333333;
  }
  /* Workspace header */
  #workspaceHeader {
    background-color: #1e1e1e;
    border-bottom: 1px solid #333333;
  }
  #workspaceProviderLabel {
    color: #e0e0e0;
    background: transparent;
    border: none;
  }
  #workspaceProviderIcon {
    background: transparent;
    border: none;
  }
  /* Empty state */
  #emptyState {
    background-color: #161616;
    color: #e0e0e0;
  }
  #emptyState QLabel {
    background: transparent;
    border: none;
  }
  #emptyStateTitle {
    color: #e0e0e0;
    font-size: 20pt;
    font-weight: 600;
  }
  #emptyStateSubtitle {
    color: #a0a0a0;
    font-size: 11pt;
  }
  #emptyStateProviderFrame {
    background: transparent;
    border: none;
  }
  #emptyStateProviderFrame {
    background-color: #242424;
    border: 1px solid #333333;
    border-radius: 10px;
  }
  #emptyStateProviderButton {
    background-color: #2a2a2a;
    color: #e0e0e0;
    border: 1px solid #333333;
    border-radius: 8px;
    padding: 10px;
    text-align: center;
  }
  #emptyStateProviderButton:hover {
    background-color: #333333;
    border-color: #4a4a4a;
    color: #ffffff;
  }
  #emptyStateProviderButton:pressed {
    background-color: #3d3d3d;
    border-color: #4a4a4a;
  }
  #emptyStateProviderButton:focus {
    border: 2px solid #78aeed;
    outline: none;
  }
  /* Generic controls */
  QToolButton {
    background-color: #242424;
    color: #e0e0e0;
    border: 1px solid #333333;
    border-radius: 6px;
    padding: 6px 14px;
    font-weight: 500;
  }
  QToolButton:hover {
    background-color: #2a2a2a;
    border-color: #4a4a4a;
  }
  QToolButton:pressed {
    background-color: #333333;
  }
  QToolButton:disabled {
    background-color: #1e1e1e;
    color: #5e5c64;
    border-color: #2a2a2a;
  }
  QToolButton:focus {
    border: 2px solid #78aeed;
    padding: 5px 13px;
  }
  QPushButton {
    background-color: #242424;
    color: #e0e0e0;
    border: 1px solid #333333;
    border-radius: 6px;
    padding: 6px 16px;
    font-weight: 500;
  }
  QPushButton:hover {
    background-color: #2a2a2a;
    border-color: #4a4a4a;
  }
  QPushButton:pressed {
    background-color: #333333;
  }
  QPushButton:disabled {
    background-color: #1e1e1e;
    color: #5e5c64;
    border-color: #2a2a2a;
  }
  QPushButton:focus {
    border: 2px solid #78aeed;
    padding: 5px 15px;
  }
  QPushButton[destructive="true"] {
    background-color: #2a1a1a;
    color: #e01b24;
    border: 1px solid #4a2a2a;
  }
  QPushButton[destructive="true"]:hover {
    background-color: #332222;
    border-color: #e01b24;
  }
  QPushButton[destructive="true"]:pressed {
    background-color: #3d2a2a;
  }
  QPushButton[default="true"] {
    background-color: #78aeed;
    color: #161616;
    border: none;
  }
  QPushButton[default="true"]:hover {
    background-color: #9cc3f3;
  }
  QPushButton[default="true"]:pressed {
    background-color: #5e9bd8;
  }
  QLineEdit, QComboBox, QSpinBox {
    background-color: #242424;
    color: #e0e0e0;
    border: 1px solid #333333;
    border-radius: 6px;
    padding: 6px 10px;
    selection-background-color: #78aeed;
    selection-color: #161616;
  }
  QLineEdit:focus, QComboBox:focus, QSpinBox:focus {
    border: 2px solid #78aeed;
    padding: 5px 9px;
  }
  QListWidget {
    background-color: #1e1e1e;
    color: #e0e0e0;
    border: 1px solid #333333;
    border-radius: 8px;
    padding: 4px;
    outline: none;
  }
  QListWidget::item {
    padding: 8px 12px;
    border-radius: 6px;
    border: none;
    margin: 1px 2px;
  }
  QListWidget::item:selected {
    background-color: #2a3b5c;
    color: #e0e0e0;
    border: 1px solid #78aeed;
  }
  QListWidget::item:hover:!selected {
    background-color: #242424;
  }
  QListWidget::item:focus {
    border: 2px solid #78aeed;
  }
  QStatusBar {
    background-color: #161616;
    color: #a0a0a0;
    border: none;
    border-top: 1px solid #333333;
    padding: 4px 8px;
  }
  QScrollBar:vertical {
    background: transparent;
    width: 10px;
    margin: 0;
    border: none;
  }
  QScrollBar::handle:vertical {
    background: #4a4a4a;
    border-radius: 5px;
    min-height: 30px;
    margin: 2px;
  }
  QScrollBar::handle:vertical:hover {
    background: #606060;
  }
  QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical,
  QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical {
    background: none;
    height: 0;
  }
  QScrollBar:horizontal {
    background: transparent;
    height: 10px;
    margin: 0;
    border: none;
  }
  QScrollBar::handle:horizontal {
    background: #4a4a4a;
    border-radius: 5px;
    min-width: 30px;
    margin: 2px;
  }
  QScrollBar::handle:horizontal:hover {
    background: #606060;
  }
  QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal,
  QScrollBar::add-page:horizontal, QScrollBar::sub-page:horizontal {
    background: none;
    width: 0;
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
