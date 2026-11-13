#include "Theme.h"

#include <QApplication>
#include <QColor>
#include <QGuiApplication>
#include <QPalette>
#include <QStyleHints>

namespace Theme {

namespace {

// Light theme semantic colors
const char* kLightStyleSheet = R"(
  /* Color palette (light) */
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
  /* Browser workspace (QStackedWidget) */
  #browserWorkspace {
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
    background-color: #f8f9fa;
    border-right: 1px solid #dee2e6;
  }
  #sidebarHeader {
    background-color: #f8f9fa;
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
    background-color: #f8f9fa;
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
  #providerList QWidget {
    background: transparent;
  }
  #providerList::viewport, #tabList::viewport {
    background: transparent;
  }
  #providerScrollArea {
    background: transparent;
    border: none;
  }
  #providerStrip {
    background: transparent;
    border: none;
  }
  #providerStripButton {
    background: transparent;
    border: 1px solid transparent;
    border-radius: 10px;
    padding: 2px;
  }
  #providerStripButton:hover {
    background-color: #e9ecef;
    border-color: #dee2e6;
  }
  #providerStripButton[selected="true"] {
    background-color: #e3f2fd;
    border-color: #3584e4;
  }
  #collapsedProviderButton[selected="true"] {
    background-color: #e3f2fd;
    border-color: #3584e4;
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
    background-color: #ffffff;
    border: 1px solid #dee2e6;
    border-radius: 8px;
  }
  #tabRowWidget:hover {
    background-color: #e9ecef;
    border-color: #adb5bd;
  }
  #tabRowWidget[selected="true"] {
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
  #sidebarExpandedContainer, #collapsedRail, #providersSection, #tabsSection {
    background-color: #f8f9fa;
  }
  #sidebarDivider {
    background-color: #dee2e6;
    border: none;
  }
  #sidebarCollapseButton, #sidebarSettingsButton {
    background: transparent;
    border: none;
    border-radius: 6px;
    padding: 2px;
    color: #212529;
  }
  #sidebarCollapseButton:hover, #sidebarSettingsButton:hover {
    background-color: #e9ecef;
  }
  #sidebarCollapseButton:pressed, #sidebarSettingsButton:pressed {
    background-color: #dee2e6;
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
    padding: 3px;
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
    padding: 2px;
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
    placeholder-text-color: #868e96;
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
    alternate-background-color: #f1f3f5;
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
  QStatusBar QLabel {
    color: #6c757d;
    background: transparent;
  }
  QStatusBar::item {
    border: none;
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
  /* Browser workspace (QStackedWidget) */
  #browserWorkspace {
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
  #providerList QWidget {
    background: transparent;
  }
  #providerList::viewport, #tabList::viewport {
    background: transparent;
  }
  #providerScrollArea {
    background: transparent;
    border: none;
  }
  #providerStrip {
    background: transparent;
    border: none;
  }
  #providerStripButton {
    background: transparent;
    border: 1px solid transparent;
    border-radius: 10px;
    padding: 2px;
  }
  #providerStripButton:hover {
    background-color: #2a2a2a;
    border-color: #4a4a4a;
  }
  #providerStripButton[selected="true"] {
    background-color: #2a3b5c;
    border-color: #78aeed;
  }
  #collapsedProviderButton[selected="true"] {
    background-color: #2a3b5c;
    border-color: #78aeed;
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
  #tabRowWidget[selected="true"] {
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
    background-color: #242424;
    border-top: 1px solid #333333;
  }
  #sidebarExpandedContainer, #collapsedRail, #providersSection, #tabsSection {
    background-color: #1e1e1e;
  }
  #sidebarDivider {
    background-color: #333333;
    border: none;
  }
  #sidebarCollapseButton, #sidebarSettingsButton {
    background: transparent;
    border: none;
    border-radius: 6px;
    padding: 2px;
    color: #e0e0e0;
  }
  #sidebarCollapseButton:hover, #sidebarSettingsButton:hover {
    background-color: #2a2a2a;
  }
  #sidebarCollapseButton:pressed, #sidebarSettingsButton:pressed {
    background-color: #333333;
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
    padding: 3px;
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
    padding: 2px;
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
    placeholder-text-color: #9a9a9a;
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
    alternate-background-color: #242424;
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
  QStatusBar QLabel {
    color: #a0a0a0;
    background: transparent;
  }
  QStatusBar::item {
    border: none;
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

  const Qt::ColorScheme scheme = QGuiApplication::styleHints()->colorScheme();
  if (scheme == Qt::ColorScheme::Dark) {
    return QLatin1String(kDarkStyleSheet);
  }
  if (scheme == Qt::ColorScheme::Light) {
    return QLatin1String(kLightStyleSheet);
  }
  const bool isDark = QGuiApplication::palette().window().color().lightness() < 128;
  return isDark ? QLatin1String(kDarkStyleSheet) : QLatin1String(kLightStyleSheet);
}

Mode effectiveMode(Mode mode) {
  if (mode == Mode::Dark || mode == Mode::Light) {
    return mode;
  }
  const Qt::ColorScheme scheme = QGuiApplication::styleHints()->colorScheme();
  if (scheme == Qt::ColorScheme::Dark) {
    return Mode::Dark;
  }
  if (scheme == Qt::ColorScheme::Light) {
    return Mode::Light;
  }
  return QGuiApplication::palette().window().color().lightness() < 128 ? Mode::Dark : Mode::Light;
}

QPalette paletteForMode(Mode mode) {
  QPalette palette = QGuiApplication::palette();
  const bool dark = mode == Mode::Dark;

  palette.setColor(QPalette::Window, QColor(dark ? "#161616" : "#f8f9fa"));
  palette.setColor(QPalette::WindowText, QColor(dark ? "#e0e0e0" : "#212529"));
  palette.setColor(QPalette::Base, QColor(dark ? "#1e1e1e" : "#ffffff"));
  palette.setColor(QPalette::AlternateBase, QColor(dark ? "#242424" : "#f1f3f5"));
  palette.setColor(QPalette::Text, QColor(dark ? "#e0e0e0" : "#212529"));
  palette.setColor(QPalette::Button, QColor(dark ? "#242424" : "#ffffff"));
  palette.setColor(QPalette::ButtonText, QColor(dark ? "#e0e0e0" : "#212529"));
  palette.setColor(QPalette::Highlight, QColor(dark ? "#78aeed" : "#3584e4"));
  palette.setColor(QPalette::HighlightedText, QColor(dark ? "#161616" : "#ffffff"));
  palette.setColor(QPalette::ToolTipBase, QColor(dark ? "#242424" : "#ffffff"));
  palette.setColor(QPalette::ToolTipText, QColor(dark ? "#e0e0e0" : "#212529"));
  palette.setColor(QPalette::PlaceholderText, QColor(dark ? "#9a9a9a" : "#868e96"));
  palette.setColor(QPalette::Mid, QColor(dark ? "#333333" : "#dee2e6"));
  palette.setColor(QPalette::Dark, QColor(dark ? "#1e1e1e" : "#adb5bd"));
  palette.setColor(QPalette::Light, QColor(dark ? "#2a2a2a" : "#ffffff"));

  palette.setColor(QPalette::Disabled, QPalette::WindowText,
                   QColor(dark ? "#5e5c64" : "#adb5bd"));
  palette.setColor(QPalette::Disabled, QPalette::Text, QColor(dark ? "#5e5c64" : "#adb5bd"));
  palette.setColor(QPalette::Disabled, QPalette::ButtonText,
                   QColor(dark ? "#5e5c64" : "#adb5bd"));
  palette.setColor(QPalette::Disabled, QPalette::Highlight,
                   QColor(dark ? "#2a2a2a" : "#e9ecef"));
  palette.setColor(QPalette::Disabled, QPalette::HighlightedText,
                   QColor(dark ? "#5e5c64" : "#adb5bd"));
  return palette;
}

void applyTheme(const QString& themeName) {
  if (qApp) {
    const Mode mode = effectiveMode(modeFromString(themeName));
    qApp->setStyleSheet(styleSheetForMode(mode));
    qApp->setPalette(paletteForMode(mode));
  }
}

}  // namespace Theme
