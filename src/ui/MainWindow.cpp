#include "MainWindow.h"

#include "AppIdentity.h"
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

MainWindow::MainWindow(QWidget* parent)
  : QMainWindow(parent)
  , m_sidebar(nullptr)
  , m_workspace(nullptr)
  , m_splitter(nullptr)
  , m_fileMenu(nullptr)
  , m_helpMenu(nullptr)
  , m_statusTabInfoLabel(nullptr)
  , m_statusBadgeLabel(nullptr)
  , m_tabCounter(0)
  , m_restoringSession(false)
  , m_currentTab(nullptr) {
  setWindowTitle("AI Workspace");
  setWindowIcon(QIcon::fromTheme(AppIdentity::kIconName, QIcon(AppIdentity::kIconResource)));
  resize(1200, 800);

  ProfileManager::instance()->loadProfiles();

  setupMenuBar();
  setupCentralWidget();
  setupStatusBar();
  applyTheme();

  connect(SettingsManager::instance(), &SettingsManager::settingChanged, this,
          [this](const QString& key) {
            if (key == "theme") {
              applyTheme();
            }
          });

  connect(qApp, &QCoreApplication::aboutToQuit, this, [this]() {
    saveSidebarState();
    saveSession();
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

void MainWindow::setupCentralWidget() {
  m_splitter = new QSplitter(Qt::Horizontal, this);
  m_splitter->setChildrenCollapsible(false);
  m_splitter->setHandleWidth(1);
  m_splitter->setObjectName("mainSplitter");

  m_sidebar = new Sidebar(this);
  m_splitter->addWidget(m_sidebar);

  m_workspace = new BrowserWorkspace(this);
  m_splitter->addWidget(m_workspace);

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
  connect(m_workspace, &BrowserWorkspace::tabOrderChanged, this, &MainWindow::saveSession);
  connect(m_workspace, &BrowserWorkspace::emptyStateProviderRequested, this,
          &MainWindow::onProviderSelected);
}

void MainWindow::setupStatusBar() {
  QStatusBar* bar = statusBar();
  bar->setSizeGripEnabled(false);

  m_statusTabInfoLabel = new QLabel("Ready", this);
  m_statusTabInfoLabel->setObjectName("statusTabInfoLabel");
  bar->addWidget(m_statusTabInfoLabel);

  m_statusBadgeLabel = new QLabel(this);
  m_statusBadgeLabel->setObjectName("statusBadgeLabel");
  m_statusBadgeLabel->setAlignment(Qt::AlignCenter);
  bar->addPermanentWidget(m_statusBadgeLabel);
}

void MainWindow::applyTheme() {
  SettingsManager* settings = SettingsManager::instance();
  Theme::applyTheme(settings->theme());
  if (m_workspace) {
    m_workspace->updateTheme(settings->theme());
  }
  if (m_sidebar) {
    m_sidebar->updateTheme(settings->theme());
  }
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
  m_restoringSession = true;

  const QList<TabMetadata> stored = TabStore::load();
  if (stored.isEmpty()) {
    m_restoringSession = false;
    return;
  }

  ProviderRegistry* registry = ProviderRegistry::instance();
  ProfileManager* profileManager = ProfileManager::instance();

  for (const TabMetadata& metadata : stored) {
    Provider* provider = registry->providerById(metadata.providerId());
    if (!provider) {
      continue;
    }

    Profile* profile = profileManager->resolveProfile(metadata.providerId(), metadata.profileId());
    if (!profile) {
      continue;
    }

    const QUrl url =
        provider->isUrlAllowed(metadata.url()) ? metadata.url() : provider->url();
    const QString tabId = nextTabId();
    BrowserTab* tab = new BrowserTab(metadata.providerId(), profile->accountId(), url,
                                     LifecycleState::Unloaded, this);
    tab->setProperty("tabId", tabId);

    connect(tab, &BrowserTab::lifecycleStateChanged, this,
            [this, tabId](LifecycleState state) {
              BrowserTab* t = m_workspace->tabById(tabId);
              if (t) {
                m_sidebar->updateTabRow(tabId, tabDisplayTitle(t), state, t->hasError());
                if (t == m_currentTab) {
                  updateStatusDisplay(t);
                }
              }
            });
    connect(tab, &BrowserTab::errorChanged, this,
            [this, tabId](bool hasError) {
              BrowserTab* t = m_workspace->tabById(tabId);
              if (t) {
                m_sidebar->updateTabRow(tabId, tabDisplayTitle(t), t->lifecycleState(), hasError);
                if (t == m_currentTab) {
                  updateStatusDisplay(t);
                }
              }
            });
    connect(tab, &BrowserTab::titleChanged, this,
            [this, tabId](const QString& title) {
              BrowserTab* t = m_workspace->tabById(tabId);
              if (t) {
                m_sidebar->updateTabRow(tabId, tabDisplayTitle(t), t->lifecycleState(), t->hasError());
                if (t == m_currentTab) {
                  updateStatusDisplay(t);
                }
              }
            });
    connect(tab, &BrowserTab::urlChanged, this, [this](const QUrl&) { saveSession(); });
    connect(tab, &BrowserTab::closeRequested, this, [this, tabId]() {
      onTabClosed(tabId);
    });

    QIcon icon = providerIcon(metadata.providerId(), LifecycleState::Unloaded, false);
    m_sidebar->addTabRow(tabId, metadata.providerId(), provider->displayName(), icon,
                         LifecycleState::Unloaded, false);
    m_workspace->addTab(tab, tabId, false);

    LifecycleManager::instance()->registerTab(tab);
  }

  if (!m_workspace->hasTabs()) {
    m_restoringSession = false;
    return;
  }

  const QStringList tabIds = m_workspace->tabIds();
  const int savedIndex = qBound(0, TabStore::currentIndex(), tabIds.size() - 1);
  const QString tabId = tabIds.at(savedIndex);
  m_workspace->activateTab(tabId);
  m_currentTab = m_workspace->tabById(tabId);
  m_sidebar->selectTabRow(tabId);
  if (m_currentTab) {
    m_sidebar->setSelectedProvider(m_currentTab->providerId());
  }
  updateStatusDisplay(m_currentTab);

  m_restoringSession = false;
}

void MainWindow::saveSession() {
  if (m_restoringSession || !m_workspace) {
    return;
  }

  QList<TabMetadata> tabs;
  for (const QString& tabId : m_workspace->tabIds()) {
    BrowserTab* tab = m_workspace->tabById(tabId);
    if (!tab) {
      continue;
    }
    TabMetadata metadata = tab->metadata();
    metadata.setTitle(tabDisplayTitle(tab));
    metadata.setUrl(tab->url());
    metadata.setLifecycleState(tab->lifecycleState());
    tabs.append(metadata);
  }

  int currentIndex = m_workspace->tabIndex(m_workspace->currentTabId());
  if (currentIndex < 0) {
    currentIndex = 0;
  }
  TabStore::save(tabs, currentIndex);
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
              m_sidebar->updateTabRow(tabId, tabDisplayTitle(t), state, t->hasError());
              if (t == m_currentTab) {
                updateStatusDisplay(t);
              }
            }
          });
  connect(tab, &BrowserTab::errorChanged, this,
          [this, tabId](bool hasError) {
            BrowserTab* t = m_workspace->tabById(tabId);
            if (t) {
              m_sidebar->updateTabRow(tabId, tabDisplayTitle(t), t->lifecycleState(), hasError);
              if (t == m_currentTab) {
                updateStatusDisplay(t);
              }
            }
          });
  connect(tab, &BrowserTab::titleChanged, this,
          [this, tabId](const QString& title) {
            BrowserTab* t = m_workspace->tabById(tabId);
            if (t) {
              m_sidebar->updateTabRow(tabId, tabDisplayTitle(t), t->lifecycleState(), t->hasError());
              if (t == m_currentTab) {
                updateStatusDisplay(t);
              }
            }
          });
  connect(tab, &BrowserTab::urlChanged, this, [this](const QUrl&) { saveSession(); });
  connect(tab, &BrowserTab::closeRequested, this, [this, tabId]() {
    onTabClosed(tabId);
  });

  Provider* provider = ProviderRegistry::instance()->providerById(providerId);
  const QString displayTitle = provider ? provider->displayName() : providerId;
  Q_UNUSED(title)

  QIcon icon = providerIcon(providerId, LifecycleState::Unloaded, false);
  m_sidebar->addTabRow(tabId, providerId, displayTitle, icon,
                       LifecycleState::Unloaded, false);
  m_workspace->addTab(tab, tabId);

  LifecycleManager::instance()->registerTab(tab);

  return tab;
}

QString MainWindow::nextTabId() {
  return QString("tab-%1").arg(++m_tabCounter);
}

void MainWindow::closeEvent(QCloseEvent* event) {
  saveSidebarState();
  saveSession();
  QMainWindow::closeEvent(event);
}

QString MainWindow::tabDisplayTitle(BrowserTab* tab) const {
  if (!tab) {
    return {};
  }
  Provider* provider = ProviderRegistry::instance()->providerById(tab->providerId());
  return provider ? provider->displayName() : tab->providerId();
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
  m_sidebar->setSelectedProvider(tab->providerId());
  updateStatusDisplay(tab);
  saveSession();
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
  m_currentTab = nextTab;
  updateStatusDisplay(nextTab);
  if (nextTab) {
    m_sidebar->setSelectedProvider(nextTab->providerId());
    LifecycleManager::instance()->onTabActivated(nextTab);
  } else {
    m_sidebar->setSelectedProvider({});
  }
  saveSession();
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
  QList<int> sizes = m_splitter->sizes();
  if (sizes.size() >= 2) {
    int total = sizes[0] + sizes[1];
    sizes[0] = width;
    sizes[1] = total - width;
    m_splitter->setSizes(sizes);
  }
}

void MainWindow::onSidebarCollapsedChanged(bool collapsed) {
  Q_UNUSED(collapsed)
}

void MainWindow::onTabActivated(BrowserTab* tab, const QString& tabId) {
  Q_UNUSED(tabId)
  m_currentTab = tab;
  LifecycleManager::instance()->onTabActivated(tab);
  updateStatusDisplay(tab);
  m_sidebar->selectTabRow(tabId);
  if (tab) {
    m_sidebar->setSelectedProvider(tab->providerId());
  }
  saveSession();
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
                             "Provider web interfaces are embedded with Qt WebEngine.<br><br>"
                             "<a href=\"https://github.com/ChiefDenis/\">GitHub</a>")
                         .arg(QApplication::applicationVersion()));
}

void MainWindow::onQuit() {
  close();
}
