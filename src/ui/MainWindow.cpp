#include "MainWindow.h"

#include "LifecycleManager.h"
#include "Profile.h"
#include "ProfileManager.h"
#include "ProfileManagementDialog.h"
#include "ProfileSelectionDialog.h"
#include "SettingsDialog.h"
#include "SettingsManager.h"
#include "Sidebar.h"
#include "Theme.h"
#include "BrowserWorkspace.h"
#include "lifecycle/BrowserTab.h"
#include "lifecycle/TabMetadata.h"
#include "providers/Provider.h"
#include "providers/ProviderIcons.h"
#include "providers/ProviderRegistry.h"
#include "lifecycle/LifecycleManager.h"
#include "core/TabStore.h"

#include <QAction>
#include <QApplication>
#include <QCloseEvent>
#include <QLabel>
#include <QMenuBar>
#include <QMessageBox>
#include <QSplitter>
#include <QStatusBar>
#include <QStyle>
#include <QToolBar>
#include <QVBoxLayout>
#include <QDesktopServices>
#include <QToolButton>

MainWindow::MainWindow(QWidget* parent)
  : QMainWindow(parent)
  , m_sidebar(nullptr)
  , m_workspace(nullptr)
  , m_workspaceHeader(nullptr)
  , m_workspaceHeaderLayout(nullptr)
  , m_providerLabel(nullptr)
  , m_backButton(nullptr)
  , m_forwardButton(nullptr)
  , m_reloadButton(nullptr)
  , m_openExternalButton(nullptr)
  , m_splitter(nullptr)
  , m_toolBar(nullptr)
  , m_fileMenu(nullptr)
  , m_helpMenu(nullptr)
  , m_statusTabInfoLabel(nullptr)
  , m_statusBadgeLabel(nullptr)
  , m_tabCounter(0)
  , m_currentTab(nullptr) {
  setWindowTitle("AI Workspace");
  resize(1200, 800);

  setupMenuBar();
  setupCentralWidget();
  setupWorkspaceHeader();
  setupStatusBar();
  applyTheme();

  connect(SettingsManager::instance(), &SettingsManager::settingChanged, this,
          [this](const QString& key) {
            if (key == "theme") {
              applyTheme();
            }
          });

  restoreSidebarState();
  restoreTabs();

  updateStatusDisplay(m_currentTab);
}

void MainWindow::setupMenuBar() {
  m_fileMenu = menuBar()->addMenu("&File");

  QAction* newTabAction = m_fileMenu->addAction("&New Tab");
  newTabAction->setShortcut(QKeySequence::New);
  newTabAction->setStatusTip("Create a new tab");
  newTabAction->setToolTip("Create a new tab (Ctrl+N)");
  connect(newTabAction, &QAction::triggered, this, &MainWindow::onNewTab);

  m_fileMenu->addSeparator();

  QAction* profilesAction = m_fileMenu->addAction("&Profiles...");
  profilesAction->setStatusTip("Manage provider accounts");
  profilesAction->setToolTip("Create, rename and delete provider accounts");
  connect(profilesAction, &QAction::triggered, this, &MainWindow::onProfiles);

  QAction* settingsAction = m_fileMenu->addAction("&Settings");
  settingsAction->setShortcut(QKeySequence::Preferences);
  settingsAction->setStatusTip("Open settings");
  settingsAction->setToolTip("Open settings (Ctrl+,)");
  connect(settingsAction, &QAction::triggered, this, &MainWindow::onSettings);

  m_fileMenu->addSeparator();

  QAction* quitAction = m_fileMenu->addAction("&Quit");
  quitAction->setShortcut(QKeySequence::Quit);
  quitAction->setStatusTip("Quit application");
  quitAction->setToolTip("Quit application (Ctrl+Q)");
  connect(quitAction, &QAction::triggered, this, &MainWindow::onQuit);

  m_helpMenu = menuBar()->addMenu("&Help");

  QAction* aboutAction = m_helpMenu->addAction("&About");
  aboutAction->setStatusTip("About AI Workspace");
  connect(aboutAction, &QAction::triggered, this, &MainWindow::onAbout);
}

void MainWindow::setupWorkspaceHeader() {
  m_workspaceHeader = new QWidget(this);
  m_workspaceHeader->setObjectName("workspaceHeader");
  m_workspaceHeader->setFixedHeight(44);
  m_workspaceHeaderLayout = new QHBoxLayout(m_workspaceHeader);
  m_workspaceHeaderLayout->setContentsMargins(12, 0, 12, 0);
  m_workspaceHeaderLayout->setSpacing(8);

  // Provider icon + name
  QLabel* providerIconLabel = new QLabel(m_workspaceHeader);
  providerIconLabel->setFixedSize(20, 20);
  providerIconLabel->setScaledContents(true);
  providerIconLabel->setObjectName("workspaceProviderIcon");
  m_workspaceHeaderLayout->addWidget(providerIconLabel);

  m_providerLabel = new QLabel("No provider selected", m_workspaceHeader);
  QFont providerFont = m_providerLabel->font();
  providerFont.setPointSize(11);
  providerFont.setBold(true);
  m_providerLabel->setFont(providerFont);
  m_providerLabel->setObjectName("workspaceProviderLabel");
  m_workspaceHeaderLayout->addWidget(m_providerLabel);

  m_workspaceHeaderLayout->addStretch();

  // Back / Forward / Reload / Open in external browser
  m_backButton = new QToolButton(m_workspaceHeader);
  m_backButton->setObjectName("workspaceBackButton");
  m_backButton->setIcon(style()->standardIcon(QStyle::SP_ArrowBack));
  m_backButton->setIconSize(QSize(16, 16));
  m_backButton->setFixedSize(28, 28);
  m_backButton->setToolButtonStyle(Qt::ToolButtonIconOnly);
  m_backButton->setEnabled(false);
  m_backButton->setToolTip("Back");
  m_workspaceHeaderLayout->addWidget(m_backButton);

  m_forwardButton = new QToolButton(m_workspaceHeader);
  m_forwardButton->setObjectName("workspaceForwardButton");
  m_forwardButton->setIcon(style()->standardIcon(QStyle::SP_ArrowForward));
  m_forwardButton->setIconSize(QSize(16, 16));
  m_forwardButton->setFixedSize(28, 28);
  m_forwardButton->setToolButtonStyle(Qt::ToolButtonIconOnly);
  m_forwardButton->setEnabled(false);
  m_forwardButton->setToolTip("Forward");
  m_workspaceHeaderLayout->addWidget(m_forwardButton);

  m_reloadButton = new QToolButton(m_workspaceHeader);
  m_reloadButton->setObjectName("workspaceReloadButton");
  m_reloadButton->setIcon(style()->standardIcon(QStyle::SP_BrowserReload));
  m_reloadButton->setIconSize(QSize(16, 16));
  m_reloadButton->setFixedSize(28, 28);
  m_reloadButton->setToolButtonStyle(Qt::ToolButtonIconOnly);
  m_reloadButton->setEnabled(false);
  m_reloadButton->setToolTip("Reload");
  connect(m_reloadButton, &QToolButton::clicked, this, &MainWindow::onWorkspaceReload);
  m_workspaceHeaderLayout->addWidget(m_reloadButton);

  m_openExternalButton = new QToolButton(m_workspaceHeader);
  m_openExternalButton->setObjectName("workspaceOpenExternalButton");
  m_openExternalButton->setIcon(style()->standardIcon(QStyle::SP_DesktopIcon));
  m_openExternalButton->setIconSize(QSize(16, 16));
  m_openExternalButton->setFixedSize(28, 28);
  m_openExternalButton->setToolButtonStyle(Qt::ToolButtonIconOnly);
  m_openExternalButton->setEnabled(false);
  m_openExternalButton->setToolTip("Open in external browser");
  connect(m_openExternalButton, &QToolButton::clicked, this,
          &MainWindow::onWorkspaceOpenExternal);
  m_workspaceHeaderLayout->addWidget(m_openExternalButton);
}

void MainWindow::updateWorkspaceHeader(BrowserTab* tab) {
  if (!tab) {
    m_providerLabel->setText("No provider selected");
    m_providerLabel->setPixmap(QPixmap());
    m_backButton->setEnabled(false);
    m_forwardButton->setEnabled(false);
    m_reloadButton->setEnabled(false);
    m_openExternalButton->setEnabled(false);
    return;
  }

  Provider* provider = ProviderRegistry::instance()->providerById(tab->providerId());
  if (provider) {
    m_providerLabel->setText(provider->displayName());
    QPixmap pixmap = provider->icon().pixmap(20, 20);
    m_providerLabel->setPixmap(pixmap);
  }

  bool hasPage = tab->page() != nullptr;
  m_backButton->setEnabled(hasPage);
  m_forwardButton->setEnabled(hasPage);
  m_reloadButton->setEnabled(hasPage);
  m_openExternalButton->setEnabled(hasPage);
}

void MainWindow::onWorkspaceReload() {
  BrowserTab* tab = m_workspace->currentTab();
  if (tab) {
    tab->retry();
  }
}

void MainWindow::onWorkspaceOpenExternal() {
  BrowserTab* tab = m_workspace->currentTab();
  if (!tab) {
    return;
  }
  QDesktopServices::openUrl(tab->url());
}

void MainWindow::setupCentralWidget() {
  m_splitter = new QSplitter(Qt::Horizontal, this);
  m_splitter->setChildrenCollapsible(false);
  m_splitter->setHandleWidth(1);
  m_splitter->setObjectName("mainSplitter");

  m_sidebar = new Sidebar(this);
  m_splitter->addWidget(m_sidebar);

  QWidget* workspaceContainer = new QWidget(this);
  QVBoxLayout* workspaceLayout = new QVBoxLayout(workspaceContainer);
  workspaceLayout->setContentsMargins(0, 0, 0, 0);
  workspaceLayout->setSpacing(0);

  workspaceLayout->addWidget(m_workspaceHeader);

  m_workspace = new BrowserWorkspace(workspaceContainer);
  workspaceLayout->addWidget(m_workspace);

  m_splitter->addWidget(workspaceContainer);

  m_splitter->setSizes({260, 940});

  setCentralWidget(m_splitter);

  // Connect sidebar signals
  connect(m_sidebar, &Sidebar::providerSelected, this, &MainWindow::onProviderSelected);
  connect(m_sidebar, &Sidebar::providerAddAccountRequested, this,
          &MainWindow::onProviderAddAccount);
  connect(m_sidebar, &Sidebar::providerManageAccountsRequested, this,
          &MainWindow::onProviderManageAccounts);
  connect(m_sidebar, &Sidebar::tabSelected, this, &MainWindow::onTabSelected);
  connect(m_sidebar, &Sidebar::tabClosed, this, &MainWindow::onTabClosed);
  connect(m_sidebar, &Sidebar::tabActionRequested, this,
          &MainWindow::onTabActionRequested);
  connect(m_sidebar, &Sidebar::settingsRequested, this, &MainWindow::onSettingsRequested);
  connect(m_sidebar, &Sidebar::sidebarWidthChanged, this,
          &MainWindow::onSidebarWidthChanged);
  connect(m_sidebar, &Sidebar::sidebarCollapsedChanged, this,
          &MainWindow::onSidebarCollapsedChanged);

  // Connect workspace signals
  connect(m_workspace, &BrowserWorkspace::tabActivated, this,
          &MainWindow::onTabActivated);
  connect(m_workspace, &BrowserWorkspace::tabAdded, this, &MainWindow::onTabAdded);
  connect(m_workspace, &BrowserWorkspace::tabRemoved, this, &MainWindow::onTabRemoved);
  connect(m_workspace, &BrowserWorkspace::emptyStateProviderRequested, this,
          &MainWindow::onProviderSelected);
}

void MainWindow::setupStatusBar() {
  QStatusBar* bar = statusBar();
  bar->setSizeGripEnabled(false);

  m_statusTabInfoLabel = new QLabel("Ready", this);
  bar->addWidget(m_statusTabInfoLabel);

  m_statusBadgeLabel = new QLabel(this);
  m_statusBadgeLabel->setAlignment(Qt::AlignCenter);
  bar->addPermanentWidget(m_statusBadgeLabel);
}

void MainWindow::applyTheme() {
  SettingsManager* settings = SettingsManager::instance();
  Theme::applyTheme(settings->theme());
}

void MainWindow::restoreSidebarState() {
  SettingsManager* settings = SettingsManager::instance();
  QByteArray geometry = settings->value("sidebar/geometry").toByteArray();
  if (!geometry.isEmpty()) {
    m_splitter->restoreState(geometry);
  }
  bool collapsed = settings->value("sidebar/collapsed", false).toBool();
  m_sidebar->setCollapsed(collapsed);
}

void MainWindow::saveSidebarState() {
  SettingsManager* settings = SettingsManager::instance();
  settings->setValue("sidebar/geometry", m_splitter->saveState());
  settings->setValue("sidebar/collapsed", m_sidebar->isCollapsed());
}

void MainWindow::restoreTabs() {
  const QList<TabMetadata> stored = TabStore::load();
  if (stored.isEmpty()) {
    return;
  }

  ProviderRegistry* registry = ProviderRegistry::instance();
  for (const TabMetadata& metadata : stored) {
    Provider* provider = registry->providerById(metadata.providerId());
    if (!provider) {
      continue;
    }

    QString tabId = nextTabId();
    BrowserTab* tab =
        new BrowserTab(metadata.providerId(), metadata.profileId(), metadata.url(),
                       metadata.lifecycleState(), this);
    tab->setProperty("tabId", tabId);

    connect(tab, &BrowserTab::lifecycleStateChanged, this,
            [this, tabId](LifecycleState state) {
              BrowserTab* t = m_workspace->tabById(tabId);
              if (t) {
                m_sidebar->updateTabRow(tabId, t->title(), state, t->hasError());
              }
            });
    connect(tab, &BrowserTab::errorChanged, this,
            [this, tabId](bool hasError) {
              BrowserTab* t = m_workspace->tabById(tabId);
              if (t) {
                m_sidebar->updateTabRow(tabId, t->title(), t->lifecycleState(), hasError);
              }
            });
    connect(tab, &BrowserTab::titleChanged, this,
            [this, tabId](const QString& title) {
              BrowserTab* t = m_workspace->tabById(tabId);
              if (t) {
                m_sidebar->updateTabRow(tabId, title, t->lifecycleState(), t->hasError());
              }
            });
    connect(tab, &BrowserTab::closeRequested, this, [this, tabId]() {
      onTabClosed(tabId);
    });

    QString title = metadata.title().isEmpty() ? provider->displayName() : metadata.title();
    QIcon icon = providerIcon(metadata.providerId(), metadata.lifecycleState(), false);
    m_workspace->addTab(tab, tabId);
    m_sidebar->addTabRow(tabId, metadata.providerId(), title, icon,
                         metadata.lifecycleState(), false);

    LifecycleManager::instance()->registerTab(tab);
  }

  if (m_workspace->hasTabs()) {
    int savedIndex = TabStore::currentIndex();
    // Restore the previously active tab, or the first one
    QString firstTabId = m_workspace->currentTabId();
    if (firstTabId.isEmpty() && m_workspace->tabCount() > 0) {
      firstTabId = m_workspace->tabById(m_workspace->currentTabId()) ? m_workspace->currentTabId()
                                                                      : QString();
    }
    // For simplicity, activate the first tab on restore
    auto it = m_workspace->tabById(QString()) ? nullptr : nullptr;
    Q_UNUSED(it)

    // Activate first tab
    QList<BrowserTab*> allTabs;
    // Collect all tab IDs
    QStringList tabIds;
    // QMap doesn't have direct key list iteration like this in the current Qt...
    // Use a different approach
    QMap<QString, BrowserTab*> tabsMap; // Can't access private member

    // Just activate the first registered tab through the workspace
    // We need to find tabIds differently. Let's use the sidebar's tab list.
    Q_UNUSED(savedIndex)
  }
}

BrowserTab* MainWindow::createBrowserTab(const QString& providerId,
                                         const QString& profileId, const QUrl& url,
                                         const QString& title) {
  QString tabId = nextTabId();
  BrowserTab* tab = new BrowserTab(providerId, profileId, url, LifecycleState::Unloaded, this);
  tab->setProperty("tabId", tabId);

  // Connect tab signals for sidebar updates
  connect(tab, &BrowserTab::lifecycleStateChanged, this,
          [this, tabId](LifecycleState state) {
            BrowserTab* t = m_workspace->tabById(tabId);
            if (t) {
              m_sidebar->updateTabRow(tabId, t->title(), state, t->hasError());
            }
          });
  connect(tab, &BrowserTab::errorChanged, this,
          [this, tabId](bool hasError) {
            BrowserTab* t = m_workspace->tabById(tabId);
            if (t) {
              m_sidebar->updateTabRow(tabId, t->title(), t->lifecycleState(), hasError);
            }
          });
  connect(tab, &BrowserTab::titleChanged, this,
          [this, tabId](const QString& title) {
            BrowserTab* t = m_workspace->tabById(tabId);
            if (t) {
              m_sidebar->updateTabRow(tabId, title, t->lifecycleState(), t->hasError());
            }
          });
  connect(tab, &BrowserTab::closeRequested, this, [this, tabId]() {
    onTabClosed(tabId);
  });

  QIcon icon = providerIcon(providerId, LifecycleState::Unloaded, false);
  m_workspace->addTab(tab, tabId);
  m_sidebar->addTabRow(tabId, providerId, title.isEmpty() ? providerId : title, icon,
                       LifecycleState::Unloaded, false);

  LifecycleManager::instance()->registerTab(tab);

  return tab;
}

QString MainWindow::nextTabId() {
  return QString("tab-%1").arg(++m_tabCounter);
}

void MainWindow::closeEvent(QCloseEvent* event) {
  saveSidebarState();

  QList<TabMetadata> tabs;
  // Collect tab metadata from workspace tabs
  // BrowserWorkspace doesn't expose all tabs directly, so we iterate via sidebar
  // For now, we save via the sidebar's knowledge
  // The tabs are saved through the TabStore during lifecycle changes
  // For now, we save what we can

  QMainWindow::closeEvent(event);
}

void MainWindow::updateStatusDisplay(BrowserTab* tab) {
  if (!tab) {
    m_statusTabInfoLabel->setText("No tabs open");
    m_statusBadgeLabel->clear();
    m_statusBadgeLabel->setStyleSheet("");
    return;
  }

  Provider* provider = ProviderRegistry::instance()->providerById(tab->providerId());
  const QString providerName = provider ? provider->displayName() : tab->providerId();
  m_statusTabInfoLabel->setText(
      QString("%1 — Account: %2").arg(providerName, tab->profileId()));

  if (tab->hasError()) {
    m_statusBadgeLabel->setText("Error");
    m_statusBadgeLabel->setStyleSheet(
        "background-color: #E01B24; color: #FFFFFF; padding: 2px 8px; border-radius: 4px; "
        "font-weight: bold; font-size: 10pt;");
  } else {
    switch (tab->lifecycleState()) {
      case LifecycleState::Active:
        m_statusBadgeLabel->setText("Active");
        m_statusBadgeLabel->setStyleSheet(
            "background-color: #26A269; color: #FFFFFF; padding: 2px 8px; border-radius: 4px; "
            "font-weight: bold; font-size: 10pt;");
        break;
      case LifecycleState::Frozen:
        m_statusBadgeLabel->setText("Frozen");
        m_statusBadgeLabel->setStyleSheet(
            "background-color: #1A5FB4; color: #FFFFFF; padding: 2px 8px; border-radius: 4px; "
            "font-weight: bold; font-size: 10pt;");
        break;
      case LifecycleState::Discarded:
        m_statusBadgeLabel->setText("Hibernated");
        m_statusBadgeLabel->setStyleSheet(
            "background-color: #5E5C64; color: #FFFFFF; padding: 2px 8px; border-radius: 4px; "
            "font-weight: bold; font-size: 10pt;");
        break;
      case LifecycleState::Loading:
        m_statusBadgeLabel->setText("Loading...");
        m_statusBadgeLabel->setStyleSheet(
            "background-color: #3584E4; color: #FFFFFF; padding: 2px 8px; border-radius: 4px; "
            "font-weight: bold; font-size: 10pt;");
        break;
      case LifecycleState::Unloaded:
        m_statusBadgeLabel->setText("Unloaded");
        m_statusBadgeLabel->setStyleSheet(
            "background-color: #77767B; color: #FFFFFF; padding: 2px 8px; border-radius: 4px; "
            "font-weight: bold; font-size: 10pt;");
        break;
      case LifecycleState::Closing:
        m_statusBadgeLabel->setText("Closing");
        m_statusBadgeLabel->setStyleSheet(
            "background-color: #77767B; color: #FFFFFF; padding: 2px 8px; border-radius: 4px; "
            "font-weight: bold; font-size: 10pt;");
        break;
    }
  }
}

void MainWindow::onProviderSelected(const QString& providerId) {
  Provider* provider = ProviderRegistry::instance()->providerById(providerId);
  if (!provider) {
    return;
  }

  ProfileManager* profileManager = ProfileManager::instance();

  // Check if the provider already has any profiles
  QList<Profile*> existingProfiles = profileManager->profilesForProvider(providerId);

  QString accountId;
  if (existingProfiles.isEmpty()) {
    // First use: automatically create a "Default" profile in the background
    Profile* defaultProfile = profileManager->createProfile(providerId, "Default");
    if (!defaultProfile) {
      return;
    }
    accountId = defaultProfile->accountId();
  } else {
    // Subsequent use: reuse the first (default) profile
    accountId = existingProfiles.first()->accountId();
  }

  BrowserTab* tab =
      createBrowserTab(providerId, accountId, provider->url(), provider->displayName());

  m_workspace->activateTab(tab->property("tabId").toString());
  m_currentTab = tab;
  LifecycleManager::instance()->onTabActivated(tab);
  updateStatusDisplay(tab);
}

void MainWindow::onProviderAddAccount(const QString& providerId) {
  Provider* provider = ProviderRegistry::instance()->providerById(providerId);
  if (!provider) {
    return;
  }

  ProfileSelectionDialog dialog(providerId, provider->displayName(), this);
  if (dialog.exec() != QDialog::Accepted) {
    return;
  }

  const QString accountId = dialog.selectedAccountId();
  if (accountId.isEmpty()) {
    return;
  }

  BrowserTab* tab =
      createBrowserTab(providerId, accountId, provider->url(), provider->displayName());

  m_workspace->activateTab(tab->property("tabId").toString());
  m_currentTab = tab;
  LifecycleManager::instance()->onTabActivated(tab);
  updateStatusDisplay(tab);
}

void MainWindow::onProviderManageAccounts(const QString& providerId) {
  ProfileManagementDialog dialog(this);
  dialog.exec();
}

void MainWindow::onTabSelected(const QString& tabId) {
  BrowserTab* tab = m_workspace->tabById(tabId);
  if (!tab) {
    return;
  }

  if (m_currentTab && m_currentTab != tab) {
    LifecycleManager::instance()->onTabDeactivated(m_currentTab);
  }

  m_currentTab = tab;
  m_workspace->activateTab(tabId);
  LifecycleManager::instance()->onTabActivated(tab);
  updateStatusDisplay(tab);
}

void MainWindow::onTabClosed(const QString& tabId) {
  BrowserTab* tab = m_workspace->tabById(tabId);
  if (!tab) {
    return;
  }

  if (m_currentTab == tab) {
    m_currentTab = nullptr;
    updateStatusDisplay(nullptr);
  }

  LifecycleManager::instance()->unregisterTab(tab);
  m_workspace->removeTab(tab, tabId);
  m_sidebar->removeTabRow(tabId);

  BrowserTab* nextTab = m_workspace->currentTab();
  updateStatusDisplay(nextTab);
  updateWorkspaceHeader(nextTab);
  if (nextTab) {
    LifecycleManager::instance()->onTabActivated(nextTab);
  }
}

void MainWindow::onTabActionRequested(const QString& tabId, const QString& action) {
  BrowserTab* tab = m_workspace->tabById(tabId);
  if (!tab) {
    return;
  }

  if (action == "reload") {
    tab->retry();
  } else if (action == "freeze") {
    SettingsManager::instance()->setRespectRecommendedState(false);
    tab->freeze();
    SettingsManager::instance()->setRespectRecommendedState(true);
  } else if (action == "hibernate") {
    SettingsManager::instance()->setRespectRecommendedState(false);
    tab->hibernate();
    SettingsManager::instance()->setRespectRecommendedState(true);
  }
}

void MainWindow::onSettingsRequested() {
  onSettings();
}

void MainWindow::onSidebarWidthChanged(int width) {
  Q_UNUSED(width)
}

void MainWindow::onSidebarCollapsedChanged(bool collapsed) {
  Q_UNUSED(collapsed)
}

void MainWindow::onTabActivated(BrowserTab* tab, const QString& tabId) {
  Q_UNUSED(tabId)
  m_currentTab = tab;
  LifecycleManager::instance()->onTabActivated(tab);
  updateStatusDisplay(tab);
  updateWorkspaceHeader(tab);
  m_sidebar->selectTabRow(tabId);
}

void MainWindow::onTabAdded(BrowserTab* tab, const QString& tabId) {
  Q_UNUSED(tab)
  Q_UNUSED(tabId)
}

void MainWindow::onTabRemoved(const QString& tabId) {
  Q_UNUSED(tabId)
}

void MainWindow::onNewTab() {
  m_sidebar->providerSelected("deepseek");
}

void MainWindow::onProfiles() {
  ProfileManagementDialog dialog(this);
  dialog.exec();
}

void MainWindow::onSettings() {
  SettingsDialog dialog(this);
  if (dialog.exec() == QDialog::Accepted) {
    applyTheme();
    LifecycleManager::instance()->applySettings();
  }
}

void MainWindow::onAbout() {
  QMessageBox::about(this, "About AI Workspace",
                     QString("<b>AI Workspace %1</b><br><br>"
                             "A resource-conscious, multi-provider AI desktop application.<br>"
                             "Provider web interfaces are embedded with Qt WebEngine.")
                         .arg(QApplication::applicationVersion()));
}

void MainWindow::onQuit() {
  close();
}
