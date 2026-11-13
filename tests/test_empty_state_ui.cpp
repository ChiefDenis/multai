#include <QtTest/QtTest>
#include <QStandardPaths>
#include <QToolButton>
#include <QListWidget>

#include "BrowserWorkspace.h"
#include "MainWindow.h"
#include "Sidebar.h"

class TestEmptyState : public QObject {
  Q_OBJECT
private slots:
  void initTestCase();
  void testEmptyStateHasProviderShortcuts();
  void testAllNineProvidersHaveShortcutButtons();
  void testProviderButtonsHaveVisibleContrast();
  void testWorkspaceHasNoProviderHeader();
};

void TestEmptyState::initTestCase() {
  qputenv("QT_QPA_PLATFORM", "offscreen");
  QStandardPaths::setTestModeEnabled(true);
}

void TestEmptyState::testEmptyStateHasProviderShortcuts() {
  BrowserWorkspace workspace;
  QVERIFY(!workspace.hasTabs());

  QList<QToolButton*> buttons = workspace.findChildren<QToolButton*>(
      QString("emptyStateProviderButton"));
  QVERIFY(!buttons.isEmpty());
}

void TestEmptyState::testAllNineProvidersHaveShortcutButtons() {
  BrowserWorkspace workspace;
  QList<QToolButton*> buttons = workspace.findChildren<QToolButton*>(
      QString("emptyStateProviderButton"));
  QCOMPARE(buttons.size(), 9);
}

void TestEmptyState::testProviderButtonsHaveVisibleContrast() {
  BrowserWorkspace workspace;
  QList<QToolButton*> buttons = workspace.findChildren<QToolButton*>(
      QString("emptyStateProviderButton"));
  QVERIFY(!buttons.isEmpty());

  // Each button should have a visible icon and text
  for (QToolButton* btn : buttons) {
    QVERIFY(!btn->icon().isNull());
    QVERIFY(!btn->text().isEmpty());
  }
}

void TestEmptyState::testWorkspaceHasNoProviderHeader() {
  MainWindow window;
  QVERIFY(window.findChild<QWidget*>("workspaceHeader") == nullptr);
  QVERIFY(window.findChild<QWidget*>("workspaceProviderIcon") == nullptr);
  QVERIFY(window.findChild<QWidget*>("workspaceProviderLabel") == nullptr);
}

QTEST_MAIN(TestEmptyState)
#include "test_empty_state_ui.moc"
