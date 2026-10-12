# AI Workspace - Architecture Design

## Document Information

- **Version**: 1.0
- **Date**: 2026-10-10
- **Status**: Initial Draft
- **Related Documents**: SPEC.md, AGENTS.md

## 1. Architecture Overview

### 1.1 High-Level Architecture

The application follows a layered architecture with clear separation of concerns:

```
┌─────────────────────────────────────────────────────────────┐
│                         UI Layer                             │
│  (Qt Widgets - MainWindow, TabWidget, Dialogs, Settings)    │
└─────────────────────────────────────────────────────────────┘
                              ↓
┌─────────────────────────────────────────────────────────────┐
│                    Application Layer                         │
│  (TabManager, LifecycleManager, ProfileManager, Provider)   │
└─────────────────────────────────────────────────────────────┘
                              ↓
┌─────────────────────────────────────────────────────────────┐
│                     Browser Layer                            │
│  (BrowserTab, WebEngineView, WebEnginePage, Profile)        │
└─────────────────────────────────────────────────────────────┘
                              ↓
┌─────────────────────────────────────────────────────────────┐
│                   Qt WebEngine Layer                         │
│  (QWebEngineView, QWebEnginePage, QWebEngineProfile)         │
└─────────────────────────────────────────────────────────────┘
                              ↓
┌─────────────────────────────────────────────────────────────┐
│                   Persistence Layer                          │
│  (QSettings, SQLite, Filesystem for Profiles)              │
└─────────────────────────────────────────────────────────────┘
```

### 1.2 Key Architectural Principles

1. **Separation of Concerns**: UI, business logic, and browser integration are separate layers
2. **State Management**: Clear state machine for tab lifecycle
3. **Resource Management**: Centralised lifecycle management for resource efficiency
4. **Profile Isolation**: Each provider account uses isolated browser profiles
5. **Event-Driven**: Qt's signal/slot mechanism for loose coupling
6. **Lazy Loading**: Resources created only when needed
7. **Configuration-Driven**: Providers and settings are configurable

## 2. Component Breakdown

### 2.1 UI Layer Components

#### 2.1.1 MainWindow
- **Purpose**: Main application window
- **Responsibilities**:
  - Host the tab widget
  - Manage window state and geometry
  - Handle menu actions and global shortcuts
  - Coordinate with other managers
- **Key Signals/Slots**:
  - `newTabRequested()`
  - `settingsRequested()`
  - `quitRequested()`

#### 2.1.2 TabWidget
- **Purpose**: Container for browser tabs
- **Responsibilities**:
  - Display and manage tab bar
  - Handle tab switching
  - Handle tab creation and deletion
  - Display tab status indicators
- **Key Signals/Slots**:
  - `currentChanged(int)` → notifies LifecycleManager
  - `tabCloseRequested(int)` → initiates tab cleanup
  - `newTabRequested()` → shows provider selection dialog

#### 2.1.3 BrowserTabWidget
- **Purpose**: Individual tab content wrapper
- **Responsibilities**:
  - Wrap BrowserView
  - Display tab-specific UI (profile selector, status)
  - Handle tab-specific actions
  - Display loading progress and errors
- **Key Signals/Slots**:
  - `profileChanged(Profile*)`
  - `lifecycleStateChanged(LifecycleState)`

#### 2.1.4 ProviderSelectionDialog
- **Purpose**: Dialog for selecting provider when creating new tab
- **Responsibilities**:
  - Display available providers
  - Allow profile selection per provider
  - Confirm tab creation
- **Key Signals/Slots**:
  - `providerSelected(Provider*, Profile*)`

#### 2.1.5 SettingsDialog
- **Purpose**: Application settings configuration
- **Responsibilities**:
  - Display lifecycle settings
  - Display profile settings
  - Display appearance settings
  - Display advanced settings
  - Persist changes to QSettings
- **Key Signals/Slots**:
  - `settingsChanged()`

#### 2.1.6 ProfileManagementDialog
- **Purpose**: Profile creation and management
- **Responsibilities**:
  - List profiles per provider
  - Create new profiles
  - Rename profiles
  - Delete profiles
- **Key Signals/Slots**:
  - `profileCreated(Profile*)`
  - `profileDeleted(Profile*)`

### 2.2 Application Layer Components

#### 2.2.1 TabManager
- **Purpose**: Central manager for all tabs
- **Responsibilities**:
  - Create and destroy tabs
  - Track tab metadata (provider, profile, URL, state)
  - Persist tab configuration
  - Restore tabs on startup
  - Coordinate with LifecycleManager
- **Key Signals/Slots**:
  - `tabCreated(BrowserTab*)`
  - `tabDestroyed(BrowserTab*)`
  - `tabActivated(BrowserTab*)`
  - `tabDeactivated(BrowserTab*)`

#### 2.2.2 LifecycleManager
- **Purpose**: Manage browser lifecycle for all tabs
- **Responsibilities**:
  - Track tab lifecycle states
  - Enforce state transitions
  - Implement automatic freezing logic
  - Implement automatic hibernation logic
  - Handle memory pressure detection
  - Implement LRU selection
  - Coordinate with Qt's lifecycle API
- **Key Signals/Slots**:
  - `lifecycleStateChanged(BrowserTab*, LifecycleState)`
  - `memoryPressureDetected()`
  - `freezeDelayChanged(int)`
  - `hibernateDelayChanged(int)`

#### 2.2.3 ProfileManager
- **Purpose**: Manage browser profiles
- **Responsibilities**:
  - Create and delete profiles
  - Track profile metadata
  - Ensure profile isolation
  - Manage profile storage location
  - Handle profile persistence
- **Key Signals/Slots**:
  - `profileCreated(Profile*)`
  - `profileDeleted(Profile*)`
  - `profileRenamed(Profile*, QString)`

#### 2.2.4 ProviderRegistry
- **Purpose**: Registry of all supported providers
- **Responsibilities**:
  - Load provider definitions
  - Provide provider metadata
  - Validate provider URLs
  - Manage provider icons
- **Key Signals/Slots**:
  - None (read-only registry)

#### 2.2.5 Provider
- **Purpose**: Provider definition and configuration
- **Responsibilities**:
  - Store provider metadata (name, URL, icon)
  - Store provider-specific settings
  - Validate provider compatibility
- **Key Signals/Slots**:
  - None (data class)

#### 2.2.6 Profile
- **Purpose**: Browser profile representation
- **Responsibilities**:
  - Store profile metadata (name, provider, path)
  - Manage QWebEngineProfile instance
  - Track profile usage statistics
- **Key Signals/Slots**:
  - None (data class with Qt profile reference)

### 2.3 Browser Layer Components

#### 2.3.1 BrowserTab
- **Purpose**: Core browser tab implementation
- **Responsibilities**:
  - Manage QWebEngineView and QWebEnginePage
  - Handle navigation
  - Monitor page lifecycle state
  - Coordinate with LifecycleManager
  - Handle renderer crashes
  - Persist tab metadata
- **Key Signals/Slots**:
  - `loadStarted()`
  - `loadFinished(bool)`
  - `loadProgress(int)`
  - `renderProcessTerminated(...)`
  - `lifecycleStateChanged(QWebEnginePage::LifecycleState)`
  - `recommendedStateChanged(QWebEnginePage::LifecycleState)`

#### 2.3.2 BrowserView
- **Purpose**: Wrapper around QWebEngineView
- **Responsibilities**:
  - Embed QWebEngineView
  - Handle view-specific events
  - Manage view lifecycle
- **Key Signals/Slots**:
  - Forwards QWebEngineView signals to BrowserTab

#### 2.3.3 ProfileStorage
- **Purpose**: Manage profile data on filesystem
- **Responsibilities**:
  - Create profile directories
  - Set appropriate permissions
  - Clean up profile data on deletion
  - Measure profile size
- **Key Signals/Slots**:
  - None (utility class)

### 2.4 Persistence Layer Components

#### 2.4.1 SettingsManager
- **Purpose**: Manage QSettings
- **Responsibilities**:
  - Load and save user preferences
  - Provide typed access to settings
  - Handle default values
  - Signal setting changes
- **Key Signals/Slots**:
  - `settingChanged(QString)`

#### 2.4.2 WorkspaceDatabase
- **Purpose**: SQLite database for structured data
- **Responsibilities**:
  - Store tab metadata
  - Store navigation history
  - Store workspace configuration
  - Provide query interface
- **Key Signals/Slots**:
  - None (data access layer)

#### 2.4.3 TabMetadata
- **Purpose**: Tab metadata data structure
- **Responsibilities**:
  - Store tab identifier
  - Store provider and profile references
  - Store URL and navigation state
  - Store lifecycle state
  - Serialize/deserialize for persistence
- **Key Signals/Slots**:
  - None (data class)

## 3. Data Flow

### 3.1 Tab Creation Flow

```
User clicks "New Tab"
    ↓
TabWidget::newTabRequested()
    ↓
ProviderSelectionDialog::exec()
    ↓
User selects provider and profile
    ↓
TabManager::createTab(provider, profile)
    ↓
ProfileManager::getOrCreateProfile(provider, profileName)
    ↓
BrowserTab::create(provider, profile)
    ↓
BrowserTab::setLifecycleState(Unloaded)
    ↓
TabWidget::addTab(browserTab)
    ↓
User selects tab
    ↓
TabWidget::currentChanged(index)
    ↓
LifecycleManager::onTabActivated(browserTab)
    ↓
BrowserTab::setLifecycleState(Loading)
    ↓
BrowserTab::loadPage(provider.url)
    ↓
BrowserTab::setLifecycleState(Active)
```

### 3.2 Tab Inactivation Flow

```
User switches to another tab
    ↓
TabWidget::currentChanged(newIndex)
    ↓
LifecycleManager::onTabDeactivated(previousTab)
    ↓
LifecycleManager::startFreezeTimer(previousTab)
    ↓
[wait for freeze delay]
    ↓
LifecycleManager::freezeTab(previousTab)
    ↓
Check: Is tab eligible for freezing?
    ↓ Yes
BrowserTab::setLifecycleState(Frozen)
    ↓
BrowserTab::setQtLifecycleState(QWebEnginePage::Frozen)
    ↓
LifecycleManager::startHibernateTimer(previousTab)
```

### 3.3 Tab Hibernation Flow

```
[wait for hibernate delay OR memory pressure]
    ↓
LifecycleManager::hibernateTab(tab)
    ↓
Check: Is tab eligible for hibernation?
    ↓ Yes
BrowserTab::setLifecycleState(Discarded)
    ↓
BrowserTab::setQtLifecycleState(QWebEnginePage::Discarded)
    ↓
BrowserTab::cleanupPageAndView()
    ↓
Keep: metadata, profile, URL, navigation state
```

### 3.4 Tab Activation Flow (from Frozen)

```
User selects frozen tab
    ↓
TabWidget::currentChanged(index)
    ↓
LifecycleManager::onTabActivated(tab)
    ↓
BrowserTab::setLifecycleState(Active)
    ↓
BrowserTab::setQtLifecycleState(QWebEnginePage::Active)
    ↓
[tab becomes interactive]
```

### 3.5 Tab Activation Flow (from Discarded)

```
User selects discarded tab
    ↓
TabWidget::currentChanged(index)
    ↓
LifecycleManager::onTabActivated(tab)
    ↓
BrowserTab::setLifecycleState(Loading)
    ↓
BrowserTab::recreatePageAndView()
    ↓
BrowserTab::loadPage(savedUrl)
    ↓
BrowserTab::setLifecycleState(Active)
```

### 3.6 Tab Closure Flow

```
User clicks tab close button
    ↓
TabWidget::tabCloseRequested(index)
    ↓
TabManager::closeTab(tab)
    ↓
BrowserTab::setLifecycleState(Closing)
    ↓
BrowserTab::cleanupPageAndView()
    ↓
BrowserTab::cleanupMetadata()
    ↓
TabWidget::removeTab(index)
    ↓
TabManager::destroyTab(tab)
```

### 3.7 Renderer Crash Recovery Flow

```
QWebEnginePage::renderProcessTerminated(status, exitCode)
    ↓
BrowserTab::onRenderProcessTerminated(status, exitCode)
    ↓
BrowserTab::setLifecycleState(Loading)
    ↓
BrowserTab::reloadPage()
    ↓
Success?
    ↓ Yes
BrowserTab::setLifecycleState(Active)
    ↓ No
BrowserTab::showError("Renderer crashed, reload failed")
    ↓
User clicks "Retry"
    ↓
BrowserTab::reloadPage()
```

## 4. Component Interactions

### 4.1 Interaction Diagram

```
┌──────────────┐
│  MainWindow  │
└──────┬───────┘
       │
       ↓
┌──────────────┐
│  TabWidget   │◄──────────────────────┐
└──────┬───────┘                       │
       │                                │
       ↓                                │
┌──────────────┐         ┌──────────────┴──────────────┐
│  TabManager  │────────→│      LifecycleManager       │
└──────┬───────┘         └──────────────┬──────────────┘
       │                                │
       ↓                                ↓
┌──────────────┐         ┌──────────────┐
│ ProfileMgr   │         │ BrowserTab   │
└──────┬───────┘         └──────┬───────┘
       │                        │
       ↓                        ↓
┌──────────────┐         ┌──────────────┐
│ProviderReg   │         │WebEngineView │
└──────────────┘         └──────┬───────┘
                                 │
                                 ↓
                         ┌──────────────┐
                         │WebEnginePage │
                         └──────┬───────┘
                                │
                                ↓
                         ┌──────────────┐
                         │WebEngineProf │
                         └──────────────┘
```

### 4.2 Key Interactions

#### 4.2.1 TabManager ↔ LifecycleManager

- **TabManager** notifies **LifecycleManager** of:
  - Tab creation
  - Tab activation
  - Tab deactivation
  - Tab destruction

- **LifecycleManager** notifies **TabManager** of:
  - Lifecycle state changes
  - Required state transitions

#### 4.2.2 LifecycleManager ↔ BrowserTab

- **LifecycleManager** commands **BrowserTab** to:
  - Change lifecycle state
  - Freeze/hibernate
  - Activate

- **BrowserTab** notifies **LifecycleManager** of:
  - Qt lifecycle state changes
  - Recommended state changes
  - Load completion
  - Renderer termination

#### 4.2.3 TabManager ↔ ProfileManager

- **TabManager** requests **ProfileManager** to:
  - Get or create profile for tab
  - Delete profile when requested

- **ProfileManager** notifies **TabManager** of:
  - Profile creation
  - Profile deletion

#### 4.2.4 BrowserTab ↔ Qt WebEngine

- **BrowserTab** uses:
  - QWebEngineView for display
  - QWebEnginePage for lifecycle control
  - QWebEngineProfile for data isolation

- **BrowserTab** monitors:
  - QWebEnginePage::lifecycleStateChanged
  - QWebEnginePage::recommendedStateChanged
  - QWebEnginePage::renderProcessTerminated

## 5. Class Structure (Preliminary)

### 5.1 Core Classes

```cpp
// Application entry point
class Application : public QApplication {
public:
    Application(int &argc, char **argv);
    MainWindow* mainWindow() const;
};

// Main window
class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);
private:
    TabWidget* m_tabWidget;
    TabManager* m_tabManager;
    LifecycleManager* m_lifecycleManager;
    ProfileManager* m_profileManager;
};

// Tab widget
class TabWidget : public QTabWidget {
    Q_OBJECT
public:
    explicit TabWidget(QWidget* parent = nullptr);
    void addBrowserTab(BrowserTab* tab);
    BrowserTab* currentBrowserTab() const;
signals:
    void newTabRequested();
    void tabCloseRequested(int index);
};

// Tab manager
class TabManager : public QObject {
    Q_OBJECT
public:
    explicit TabManager(QObject* parent = nullptr);
    BrowserTab* createTab(Provider* provider, Profile* profile);
    void closeTab(BrowserTab* tab);
    void restoreTabs();
signals:
    void tabCreated(BrowserTab* tab);
    void tabDestroyed(BrowserTab* tab);
    void tabActivated(BrowserTab* tab);
    void tabDeactivated(BrowserTab* tab);
};

// Lifecycle manager
class LifecycleManager : public QObject {
    Q_OBJECT
public:
    explicit LifecycleManager(QObject* parent = nullptr);
    void onTabActivated(BrowserTab* tab);
    void onTabDeactivated(BrowserTab* tab);
    void freezeTab(BrowserTab* tab);
    void hibernateTab(BrowserTab* tab);
    void onMemoryPressure();
private:
    struct TabState {
        BrowserTab* tab;
        QDateTime lastActive;
        QTimer* freezeTimer;
        QTimer* hibernateTimer;
    };
    QMap<BrowserTab*, TabState> m_tabStates;
};

// Profile manager
class ProfileManager : public QObject {
    Q_OBJECT
public:
    explicit ProfileManager(QObject* parent = nullptr);
    Profile* getOrCreateProfile(Provider* provider, const QString& name);
    void deleteProfile(Profile* profile);
    QList<Profile*> profilesForProvider(Provider* provider) const;
signals:
    void profileCreated(Profile* profile);
    void profileDeleted(Profile* profile);
};

// Provider registry
class ProviderRegistry : public QObject {
    Q_OBJECT
public:
    static ProviderRegistry* instance();
    QList<Provider*> providers() const;
    Provider* providerById(const QString& id) const;
private:
    QMap<QString, Provider*> m_providers;
};

// Provider
class Provider : public QObject {
    Q_OBJECT
public:
    QString id() const;
    QString displayName() const;
    QUrl url() const;
    QIcon icon() const;
};

// Profile
class Profile : public QObject {
    Q_OBJECT
public:
    QString id() const;
    QString name() const;
    Provider* provider() const;
    QString path() const;
    QWebEngineProfile* webEngineProfile() const;
};

// Browser tab
class BrowserTab : public QWidget {
    Q_OBJECT
public:
    enum class LifecycleState {
        Unloaded,
        Loading,
        Active,
        Frozen,
        Discarded,
        Closing
    };
    explicit BrowserTab(Provider* provider, Profile* profile, QWidget* parent = nullptr);
    LifecycleState lifecycleState() const;
    void setLifecycleState(LifecycleState state);
    void loadPage(const QUrl& url);
    void reloadPage();
signals:
    void lifecycleStateChanged(LifecycleState state);
    void loadStarted();
    void loadFinished(bool success);
    void renderProcessTerminated(QWebEnginePage::RenderProcessTerminationStatus status, int exitCode);
private:
    Provider* m_provider;
    Profile* m_profile;
    BrowserView* m_view;
    QWebEnginePage* m_page;
    LifecycleState m_lifecycleState;
    QUrl m_savedUrl;
};
```

## 6. Module Organization

### 6.1 Directory Structure

```
src/
├── core/
│   ├── Application.cpp
│   ├── Application.h
│   ├── SettingsManager.cpp
│   └── SettingsManager.h
├── ui/
│   ├── MainWindow.cpp
│   ├── MainWindow.h
│   ├── TabWidget.cpp
│   ├── TabWidget.h
│   ├── BrowserTabWidget.cpp
│   ├── BrowserTabWidget.h
│   ├── ProviderSelectionDialog.cpp
│   ├── ProviderSelectionDialog.h
│   ├── SettingsDialog.cpp
│   ├── SettingsDialog.h
│   └── ProfileManagementDialog.cpp
│       └── ProfileManagementDialog.h
├── managers/
│   ├── TabManager.cpp
│   ├── TabManager.h
│   ├── LifecycleManager.cpp
│   ├── LifecycleManager.h
│   └── ProfileManager.cpp
│       └── ProfileManager.h
├── providers/
│   ├── ProviderRegistry.cpp
│   ├── ProviderRegistry.h
│   ├── Provider.cpp
│   ├── Provider.h
│   └── ProviderDefinitions.cpp
├── lifecycle/
│   ├── BrowserTab.cpp
│   ├── BrowserTab.h
│   ├── BrowserView.cpp
│   ├── BrowserView.h
│   ├── LifecycleState.cpp
│   └── LifecycleState.h
├── profiles/
│   ├── Profile.cpp
│   ├── Profile.h
│   ├── ProfileStorage.cpp
│   └── ProfileStorage.h
└── persistence/
    ├── WorkspaceDatabase.cpp
    ├── WorkspaceDatabase.h
    ├── TabMetadata.cpp
    └── TabMetadata.h
```

### 6.2 Module Dependencies

```
core/
  ↓ depends on
  Qt Core, Qt Widgets

ui/
  ↓ depends on
  core/, managers/, providers/, lifecycle/

managers/
  ↓ depends on
  core/, providers/, lifecycle/, profiles/

providers/
  ↓ depends on
  core/

lifecycle/
  ↓ depends on
  core/, providers/, profiles/

profiles/
  ↓ depends on
  core/, providers/

persistence/
  ↓ depends on
  core/, providers/
```

## 7. Key Design Patterns

### 7.1 Singleton Pattern

- **ProviderRegistry**: Single registry instance for all providers
- **SettingsManager**: Single settings manager for application

### 7.2 Manager Pattern

- **TabManager**: Centralised tab management
- **LifecycleManager**: Centralised lifecycle management
- **ProfileManager**: Centralised profile management

### 7.3 State Pattern

- **BrowserTab**: Different behaviour based on LifecycleState
- **LifecycleManager**: State machine for tab lifecycle

### 7.4 Observer Pattern

- Qt's signal/slot mechanism throughout
- LifecycleManager observes tab activation/deactivation
- BrowserTab observes Qt WebEngine lifecycle changes

### 7.5 Factory Pattern

- **ProfileManager**: Factory for creating profiles
- **TabManager**: Factory for creating tabs

### 7.6 Repository Pattern

- **WorkspaceDatabase**: Data access for tab metadata
- **ProviderRegistry**: Repository for provider definitions

## 8. Integration Points

### 8.1 Qt WebEngine Integration

- **QWebEngineView**: Embedded in BrowserView
- **QWebEnginePage**: Controlled by BrowserTab for lifecycle
- **QWebEngineProfile**: Managed by Profile for isolation
- **Lifecycle API**: Used by LifecycleManager via BrowserTab

### 8.2 Qt Settings Integration

- **QSettings**: Used by SettingsManager for preferences
- **Settings location**: Standard Qt settings location
- **Settings format**: INI format for portability

### 8.3 SQLite Integration

- **QtSql**: Used by WorkspaceDatabase
- **Database location**: Application data directory
- **Database schema**: Defined in WorkspaceDatabase

### 8.4 Filesystem Integration

- **Profile storage**: Separate directory in user home
- **Profile permissions**: User-only (0700)
- **Profile structure**: Follows Qt WebEngine conventions

## 9. Threading Model

### 9.1 Main Thread

- All UI operations on main thread
- Qt Widgets requires main thread
- Qt WebEngine requires main thread for view operations

### 9.2 WebEngine Processes

- Qt WebEngine creates separate renderer processes
- Each profile may have separate renderer processes
- Processes managed by Qt WebEngine internally

### 9.3 Asynchronous Operations

- Page loading is asynchronous
- Qt signals used for async events
- No explicit threading required (Qt handles it)

## 10. Error Handling Strategy

### 10.1 Qt Signal Errors

- Connect to error signals (e.g., renderProcessTerminated)
- Show user-friendly error messages
- Provide recovery options (retry, close)

### 10.2 Filesystem Errors

- Check for permissions before operations
- Show error dialog on failure
- Log errors for debugging

### 10.3 Database Errors

- Handle database connection failures
- Show error dialog on query failure
- Provide fallback to in-memory storage if needed

### 10.4 Network Errors

- Qt WebEngine handles network errors internally
- Show error page in browser view
- Allow user to retry navigation

## 11. Security Considerations

### 11.1 Profile Isolation

- Each profile in separate directory
- User-only permissions on profile directories
- No cross-profile data access

### 11.2 URL Validation

- Validate URLs before navigation
- Prevent javascript: URLs (if needed)
- Whitelist provider URLs

### 11.3 Data Protection

- No logging of sensitive data
- Secure storage of profile data
- No transmission of profile data

## 12. Performance Considerations

### 12.1 Lazy Loading

- Tabs created Unloaded, not Active
- Browser view created only when tab selected
- Profile created only when needed

### 12.2 Resource Management

- Automatic freezing of inactive tabs
- Automatic hibernation of long-inactive tabs
- Memory pressure response
- LRU selection for resource allocation

### 12.3 Caching

- Qt WebEngine handles HTTP caching
- Profile data persists across sessions
- Metadata cached in memory

## 13. Implementation Status

### Stage 1: Project Foundation (Complete)

The following components have been implemented:

#### Core Layer
- **Application**: ✅ Implemented (QApplication subclass)
  - Location: src/core/Application.h, Application.cpp
  - Sets application metadata (name, version, organization)
  - Creates and shows MainWindow

- **SettingsManager**: ✅ Implemented (QSettings wrapper with INI format)
  - Location: src/core/SettingsManager.h, SettingsManager.cpp
  - Singleton pattern
  - Provides typed access to settings
  - Signals setting changes
  - Stores theme, window geometry, window state

#### Provider Layer
- **Provider**: ✅ Implemented (data structure with id, displayName, url, icon)
  - Location: src/providers/Provider.h, Provider.cpp
  - Simple data class
  - Stores provider metadata

- **ProviderRegistry**: ✅ Implemented (singleton with all 9 providers configured)
  - Location: src/providers/ProviderRegistry.h, ProviderRegistry.cpp
  - Singleton pattern
  - Loads all 9 providers on initialization
  - Provides provider lookup by ID
  - No hardcoded UI logic - UI reads from registry

#### UI Layer
- **MainWindow**: ✅ Implemented (QMainWindow with Libadwaita-inspired design)
  - Location: src/ui/MainWindow.h, MainWindow.cpp
  - Menu bar with File and Help menus
  - Toolbar with New Tab button
  - Keyboard shortcuts (Ctrl+N, Ctrl+,, Ctrl+Q)
  - Tooltips on all controls
  - Window geometry and state persistence
  - Basic theming (light/dark/system) with CSS stylesheets
  - Libadwaita-inspired design (rounded corners, clean typography)

- **TabWidget**: ✅ Implemented (QWidget with empty-state view)
  - Location: src/ui/TabWidget.h, TabWidget.cpp
  - Note: Inherits from QWidget, not QTabWidget (browser tabs not yet implemented)
  - Shows empty state when no tabs open
  - Stacked widget to switch between empty state and tabs
  - "New Tab" button in empty state
  - Calls ProviderSelectionDialog

- **ProviderSelectionDialog**: ✅ Implemented (QDialog with provider catalogue)
  - Location: src/ui/ProviderSelectionDialog.h, ProviderSelectionDialog.cpp
  - Lists all 9 providers from ProviderRegistry
  - Search/filter functionality
  - Double-click to select
  - Tooltips on list items
  - Reads from ProviderRegistry (not hardcoded)

#### Tests
- **test_providerregistry**: ✅ Implemented
  - Location: tests/test_providerregistry.cpp
  - Tests singleton instance
  - Tests provider count (9 providers)
  - Tests provider lookup by ID
  - Tests all provider IDs present

- **test_settingsmanager**: ✅ Implemented
  - Location: tests/test_settingsmanager.cpp
  - Tests singleton instance
  - Tests value storage and retrieval
  - Tests setting reset
  - Tests theme persistence
  - Tests window geometry persistence

#### Not Yet Implemented
- TabManager (Stage 4)
- LifecycleManager (Stage 5)
- ProfileManager (Stage 3)
- BrowserTab (Stage 4)
- BrowserView (Stage 4)
- WorkspaceDatabase (future)
- All browser integration components (Stage 4+)
- Real tab functionality (QTabWidget with browser views)
- Profile creation and management
- Browser lifecycle management

## 14. Extension Points

### 14.1 New Providers

- Add provider definition to ProviderRegistry::initializeProviders()
- Add icon to resources
- No code changes required (configuration-driven)

### 14.2 New Lifecycle Policies

- Extend LifecycleManager with new policies
- Add settings for policy configuration
- Keep state machine unchanged

### 14.3 New Persistence Backends

- Abstract WorkspaceDatabase interface
- Implement new backend (e.g., JSON files)
- Swap implementation at runtime

## 15. Testing Strategy

### 15.1 Unit Tests

- Test each manager independently
- Mock Qt WebEngine for browser tests
- Test state machine transitions

### 14.2 Integration Tests

- Test manager interactions
- Test persistence operations
- Test profile management

### 14.3 UI Tests

- Qt Test for UI components
- Simulate user interactions
- Test signal/slot connections

### 14.4 End-to-End Tests

- Test complete tab lifecycle
- Test provider loading
- Test multi-account scenarios

## 15. Deployment Considerations

### 15.1 Packaging

- RPM package for Fedora
- Include Qt WebEngine dependency
- Include profile directory creation
- Include desktop file and icon

### 15.2 Installation

- Install to /usr/bin or /usr/local/bin
- Create profile directory in ~/.config/ai-workspace
- Register desktop file
- Install icons to standard locations

### 15.3 Upgrades

- Migrate settings if format changes
- Migrate database schema if needed
- Preserve user profiles across upgrades
- Handle Qt version changes gracefully
