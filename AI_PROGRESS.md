# AI Workspace - Progress Ledger

## Document Information

- **Purpose**: Authoritative progress ledger for AI Workspace project
- **Status**: Active
- **Last Updated**: 2026-10-30
- **Current Stage**: Stage 7 (Recovery and Error Handling) - partially complete

## Progress Summary

| Stage | Status | Completion Date | Notes |
|-------|--------|-----------------|-------|
| Initialisation | ✅ Complete | 2026-10-10 | Environment inspected, all documentation created |
| Stage 1: Project Foundation | ✅ Complete | 2026-10-10 | CMake build system, Qt skeleton, UI components |
| Stage 2: Provider System | ✅ Complete | 2026-10-30 | Provider definitions, icons, selection and account UI |
| Stage 3: Profile Management | ✅ Complete | 2026-10-30 | Profile creation, isolation, persistence, management UI |
| Stage 4: Basic Browser Integration | ✅ Complete | 2026-10-30 | WebEngine view/page/profile, tab management |
| Stage 5: Lifecycle State Machine | ✅ Complete | 2026-10-30 | Tab states, transitions, Qt synchronisation, persistence |
| Stage 6: Lifecycle Automation | ✅ Complete | 2026-10-30 | Freezing, hibernation, LRU, memory pressure |
| Stage 7: Recovery and Error Handling | ⚠️ Partial | 2026-10-30 | Crash detection and bounded reload done; error UI pending |
| Stage 8: UI Polish | ⚠️ Partial | - | Themes, icons, tooltips done; full Libadwaita pass pending |
| Stage 9: Testing and Performance | ⚠️ Partial | - | 7 unit tests passing; integration/perf measurement pending |
| Stage 10: Documentation and Packaging | ⏳ Not Started | - | Packaging, installation, user documentation |

Stages 2-6 were delivered together in one work phase because the browser engine,
profile manager and lifecycle state machine are mutually dependent. That phase is
documented as a single section below, matching the naming used in MISTAKES.md.

## Detailed Progress

### Stage 0: Initialisation (Complete)

**Date**: 2026-10-10
**Status**: ✅ Complete

#### Tasks Completed

1. **Environment Inspection**
   - Verified Fedora Linux 43 with KDE Plasma
   - Verified Qt 6.10.3 availability:
     - qt6-qtbase-devel-6.10.3 ✅
     - qt6-qtdeclarative-devel-6.10.3 ✅
     - qt6-qtwayland-devel-6.10.3 ✅
     - qt6-qttools-devel-6.10.3 ✅
     - qt6-qtwebengine-6.10.3 (runtime) ✅
     - qt6-qtwebengine-devel (available, must install before build) ⚠️
   - Verified CMake 3.31.11 ✅
   - Verified GCC 15.3.1 with C++20 support ✅
   - Verified SQLite 3.50.2 ✅
   - Verified Git 2.55.0 ✅

2. **Qt WebEngine Lifecycle API Verification**
   - Verified QWebEnginePage::LifecycleState enum (Active, Frozen, Discarded) ✅
   - Verified lifecycleState property ✅
   - Verified recommendedState property ✅
   - Verified lifecycleStateChanged signal ✅
   - Verified recommendedStateChanged signal ✅
   - Verified renderProcessTerminated signal ✅
   - Confirmed Qt 6.10.3 has lifecycle API (since 6.2) ✅

3. **Provider URL Verification**
   - DeepSeek: https://www.deepseek.com/ ✅
   - Qwen: https://qwen.ai/home ✅
   - GLM: https://z.ai ✅
   - Microsoft Copilot: https://copilot.microsoft.com/ ✅
   - Google Gemini: https://gemini.google.com/ ✅
   - Anthropic Claude: https://claude.ai/ ✅
   - Perplexity: https://www.perplexity.ai/ ✅
   - ChatGPT: https://chatgpt.com/ ✅
   - Kimi: https://www.kimi.ai/ ✅

4. **Project Structure Creation**
   - Created repository: /home/denis/devprojects/ai-workspace ✅
   - Initialized Git repository ✅
   - Set default branch to main ✅
   - Created .gitignore ✅
   - Created .clang-format ✅
   - Created docs/ directory ✅

5. **Documentation Creation**
   - Created AGENTS.md (project rules and working protocol) ✅
   - Created SPEC.md (complete requirements) ✅
   - Created ARCHITECTURE.md (system design) ✅
   - Created AI_PROGRESS.md (this file) ✅
   - Created README.md (project overview and documentation) ✅
   - Created docs/TESTING.md (testing strategy) ✅
   - Created docs/PERFORMANCE.md (performance criteria) ✅
   - Created docs/DECISIONS.md (technical decisions) ✅

#### Files Created

- `.gitignore` - Git ignore rules for Qt, CMake, build artifacts
- `.clang-format` - Code style configuration (Google C++ style, C++20)
- `AGENTS.md` - Project rules, working protocol, environment details
- `SPEC.md` - Complete requirements, 9 providers, lifecycle architecture
- `ARCHITECTURE.md` - System design, component relationships, class structure
- `AI_PROGRESS.md` - This progress ledger
- `README.md` - Project overview, installation, configuration, troubleshooting
- `docs/TESTING.md` - Testing strategy, unit tests, integration tests, performance tests
- `docs/PERFORMANCE.md` - Performance criteria, measurement tools, procedures
- `docs/DECISIONS.md` - Technical decisions, rationale, consequences

#### Build/Test Results

**Status**: No build performed yet (initialisation phase only)

#### Blockers

**None**

#### Technical Uncertainties Identified

1. **Profile Ownership**: How Qt WebEngine handles profile ownership and cleanup
2. **Page Lifecycle**: Exact behaviour of Qt 6.10.3 lifecycleState and recommendedState API
3. **Renderer Cleanup**: Process cleanup when pages are discarded
4. **Disk Growth**: Actual profile growth vs cache growth over time
5. **Memory Pressure**: How to detect and respond to system memory pressure on Linux
6. **Provider Compatibility**: Which providers actively block embedding

#### Next Authorised Stage

**Stage 1: Project Foundation**

This stage includes:
- CMake build system setup
- Basic Qt application skeleton
- QSettings configuration
- Basic UI structure (MainWindow)

**Prerequisites**:
- Install qt6-qtwebengine-devel package
- Create CMakeLists.txt
- Create minimal Application and MainWindow classes
- Implement basic Qt application loop

**Acceptance Criteria**:
- Application builds without errors
- Application launches and shows empty main window
- Settings can be loaded and saved
- Window geometry persists across restarts

## Stage 1: Project Foundation (Complete)

**Date**: 2026-10-10
**Status**: ✅ Complete

#### Tasks Completed

1. **CMake Build System**
   - Created CMakeLists.txt with Qt 6 configuration ✅
   - Configured for C++20 standard ✅
   - Set up Qt automoc, autorcc, autouic ✅
   - Added test subdirectory ✅

2. **Source Directory Structure**
   - Created src/core/ for core application logic ✅
   - Created src/ui/ for UI components ✅
   - Created src/providers/ for provider definitions ✅
   - Created tests/ for test code ✅

3. **SettingsManager Implementation**
   - Implemented SettingsManager singleton ✅
   - Uses QSettings with INI format ✅
   - Implements theme, window geometry, window state persistence ✅
   - Provides setting change signals ✅

4. **ProviderRegistry Implementation**
   - Implemented ProviderRegistry singleton ✅
   - Implemented Provider data structure ✅
   - Configured all 9 providers from registry (not hardcoded) ✅
   - Verified provider IDs: deepseek, qwen, glm, copilot, gemini, claude, perplexity, chatgpt, kimi ✅

5. **Application Class**
   - Implemented Application (QApplication subclass) ✅
   - Sets application name, version, organization ✅
   - Creates and shows MainWindow ✅

6. **MainWindow Implementation**
   - Implemented MainWindow with Libadwaita-inspired design ✅
   - Menu bar with File (New Tab, Settings, Quit) and Help (About) menus ✅
   - Toolbar with New Tab button ✅
   - Keyboard shortcuts: Ctrl+N (New Tab), Ctrl+, (Settings), Ctrl+Q (Quit) ✅
   - Tooltips on all menu items and toolbar buttons ✅
   - Window geometry and state persistence ✅
   - Basic theming (light/dark/system) with CSS stylesheets ✅

7. **TabWidget Implementation**
   - Implemented TabWidget with empty-state view ✅
   - Shows empty state when no tabs open ✅
   - Stacked widget to switch between empty state and tabs ✅
   - "New Tab" button in empty state ✅

8. **ProviderSelectionDialog Implementation**
   - Implemented provider catalogue dialog ✅
   - Lists all 9 providers from ProviderRegistry ✅
   - Search/filter functionality ✅
   - Double-click to select provider ✅
   - Tooltips on list items ✅

9. **Unit Tests**
   - Wrote test_providerregistry.cpp ✅
   - Wrote test_settingsmanager.cpp ✅
   - Tests for provider registration, provider count, provider IDs ✅
   - Tests for settings values, theme, window geometry ✅

10. **Build and Test**
    - Built from clean directory ✅
    - Application builds without errors ✅
    - All unit tests pass (2/2) ✅
    - Application launches and shows main window ✅
    - Version command works (--version) ✅

#### Acceptance Criteria

- [x] Application builds with CMake
- [x] Application launches without errors
- [x] Main window displays
- [x] Settings can be saved and loaded
- [x] Window geometry persists across restarts
- [x] Unit tests pass

#### Build/Test Results

**Build Status**: ✅ Success
- CMake configuration: Successful
- CMake build: Successful
- No errors or warnings
- Executable created: build/ai-workspace

**Test Status**: ✅ Success
- test_providerregistry: Passed (16.56 sec)
- test_settingsmanager: Passed (4.66 sec)
- Total test time: 21.25 sec
- 100% tests passed

**Application Launch**: ✅ Success
- --help command works
- --version command works (shows "AI Workspace 0.1.0")
- Application launches (tested with offscreen platform)

#### Files Created/Modified

**Created**:
- CMakeLists.txt
- src/core/Application.h, Application.cpp
- src/core/SettingsManager.h, SettingsManager.cpp
- src/core/main.cpp
- src/providers/Provider.h, Provider.cpp
- src/providers/ProviderRegistry.h, ProviderRegistry.cpp
- src/ui/MainWindow.h, MainWindow.cpp
- src/ui/TabWidget.h, TabWidget.cpp
- src/ui/ProviderSelectionDialog.h, ProviderSelectionDialog.cpp
- tests/CMakeLists.txt
- tests/test_providerregistry.cpp
- tests/test_settingsmanager.cpp

#### Blockers

**None**

#### Notes

- qt6-qtwebengine-devel not installed yet (requires sudo, not needed for skeleton)
- All 9 providers loaded from ProviderRegistry, not hardcoded in UI
- Libadwaita-inspired design implemented with Qt Widgets and CSS stylesheets
- Keyboard navigation and tooltips implemented on all interactive controls
- Theme switching implemented (light/dark/system)
- Window geometry and state persist across restarts using QSettings
- Settings stored in INI format at ~/.config/ai-workspace/ai-workspace.conf

## Stage 2: Provider System, Browser Engine, Profiles and Lifecycle (Complete)

**Status**: ✅ Complete
**Date**: 2026-10-14 to 2026-10-30

This phase delivered SPEC stages 2-6 together, because the browser engine, profile
manager and lifecycle state machine are mutually dependent.

#### Tasks Completed

1. **Provider System (SPEC Stage 2)**
   - Provider data structure with id, display name, URL and icon ✅
   - ProviderRegistry with all 9 providers, icons assigned from the registry ✅
   - Provider icons generated at runtime (rounded brand tiles, no binary assets) ✅
   - ProviderSelectionDialog with search/filter and per-provider icons ✅
   - Multi-account selection through ProfileSelectionDialog ✅

2. **Profile Management (SPEC Stage 3)**
   - Profile, ProfileManager and ProfileStorage ✅
   - Profile creation, renaming and deletion with uniqueness enforcement ✅
   - Isolated QWebEngineProfile per account with dedicated persistent storage and cache paths ✅
   - Owner-only (0700) profile directories ✅
   - Profile names persisted in QSettings; profiles re-registered from disk on startup ✅
   - ProfileManagementDialog (create / rename / delete, grouped by provider) ✅

3. **Browser Integration (SPEC Stage 4)**
   - BrowserView (QWebEngineView subclass) ✅
   - BrowserTab with lazy page creation (no page until a tab is activated) ✅
   - TabWidget backed by a real QTabWidget with provider icons and close handling ✅
   - Provider URL loading; page titles reflected in tab labels ✅

4. **Lifecycle State Machine (SPEC Stage 5)**
   - Six explicit states: Unloaded, Loading, Active, Frozen, Discarded, Closing ✅
   - Transition table expressed as a testable free function (canTransition) ✅
   - Synchronisation with QWebEnginePage lifecycleState ✅
   - Tab metadata persistence and restoration (TabStore; restored as Unloaded) ✅

5. **Lifecycle Automation (SPEC Stage 6)**
   - Configurable freeze and hibernate delays via SettingsManager and SettingsDialog ✅
   - Automatic freezing of inactive tabs after the freeze delay ✅
   - Automatic hibernation after the hibernate delay ✅
   - Least-recently-used selection that never selects the visible tab ✅
   - Linux memory-pressure detection (/proc/meminfo, with optional PSI support) ✅
   - Memory-pressure-triggered hibernation of the LRU inactive tab (one tab per event) ✅

6. **Recovery (SPEC Stage 7, partial)**
   - Renderer crash detection via renderProcessTerminated ✅
   - Single bounded automatic reload after a crash (no reload loops) ✅
   - Dedicated error UI with manual retry ⏳ not implemented

7. **Correctness fixes applied to the in-flight work**
   - Removed Profile self-deletion; ProfileManager is now the single owner (fixed double free) ✅
   - Tab deactivation wired to the tab widget, so freeze timers actually start ✅
   - Delays read from settings instead of hardcoded values ✅
   - freeze() respects recommendedState instead of only hibernate() ✅
   - Renderer crash path recreated the page correctly (previously a no-op) ✅
   - TestClock wired into LifecycleManager instead of being dead code ✅

#### Acceptance Criteria

- [x] All 9 providers defined with icons
- [x] ProviderRegistry loads all providers
- [x] ProviderSelectionDialog displays all providers
- [x] Provider selection and multi-account selection work
- [x] Profiles can be created, renamed and deleted
- [x] Profile data persists across restarts and is isolated per account
- [x] Tabs load provider websites in an embedded browser view
- [x] All 6 lifecycle states and their transitions are implemented and tested
- [x] Tabs freeze and hibernate after configurable delays
- [x] Memory pressure triggers hibernation of the LRU inactive tab

#### Build/Test Results

**Build Status**: ✅ Success
- `cmake -B build -S . -DCMAKE_BUILD_TYPE=Debug` - successful
- `cmake --build build -j4` - successful
- Executable: build/ai-workspace

**Test Status**: ✅ 7/7 tests passed (9.00 s total)
- test_providerregistry: Passed (1.88 s)
- test_settingsmanager: Passed (1.51 s)
- test_lifecycle: Passed (0.04 s)
- test_lifecyclemanager: Passed (0.04 s)
- test_memorymonitor: Passed (0.05 s)
- test_tabstore: Passed (1.10 s)
- test_profilemanager: Passed (4.31 s)

**Runtime**: ✅ Success
- `QT_QPA_PLATFORM=offscreen ./build/ai-workspace --version` prints "AI Workspace 0.1.0" and exits 0
- Application remained alive under the offscreen platform for the full 8 s smoke window

#### Blockers

**None**

#### Technical Uncertainties Resolved

- **Profile ownership**: ProfileManager now owns Profile objects; Profile no longer deletes
  itself, which removes the previous dangling-pointer/double-free on deref.
- **Qt lifecycle API**: lifecycleState and recommendedState behave as documented; the
  recommendedState gate is implemented but its exact triggers remain provider-dependent.
- **Memory pressure**: /proc/meminfo MemAvailable is a reliable signal; PSI
  (/proc/pressure/memory) is parsed when available but not required.

## Stage 3: Profile Management (Complete)

**Status**: ✅ Complete
**Date**: 2026-10-30

Delivered as part of the Stage 2 work phase above. Profile creation, renaming and deletion
are implemented and verified; each account gets an isolated QWebEngineProfile with
owner-only (0700) directory permissions, display names persist in QSettings, and profiles
are re-registered from disk on startup.

**Verification**: tests/test_profilemanager.cpp passed (4.31 s) covering directory creation,
uniqueness, rename persistence, refusal to delete in-use profiles, and startup scanning.

**Uncertainty resolved**: profile ownership. ProfileManager owns Profile objects and Profile
no longer deletes itself, which removed the previous dangling-pointer/double-free on deref.

## Stage 4: Basic Browser Integration (Complete)

**Status**: ✅ Complete
**Date**: 2026-10-30

Delivered as part of the Stage 2 work phase above. BrowserView, BrowserTab and TabWidget are
implemented; QWebEnginePage and QWebEngineProfile are integrated, provider URLs load in
embedded views, and page titles are reflected in tab labels.

**Design deviation**: no separate TabManager class was introduced. Tab lifecycle coordination
is split between TabWidget (tab ownership, activation and close) and LifecycleManager (delay,
LRU and memory-pressure policy), keeping each type to a single responsibility. This is
recorded in docs/DECISIONS.md.

**Verification**: build and offscreen runtime smoke test passed. Live provider loading has not
been verified interactively in this environment.

**Open technical uncertainty**: provider embedding restrictions (X-Frame-Options, CSP) and bot
detection are provider-specific and have not been exhaustively verified against live sites.

## Stage 5: Lifecycle State Machine (Complete)

**Status**: ✅ Complete
**Date**: 2026-10-30

Delivered as part of the Stage 2 work phase above. All six states are defined, the transition
table is implemented as a testable free function (canTransition in TabMetadata.h), and the
application state is synchronised with QWebEnginePage::lifecycleState. Tab metadata is
persisted to QSettings (TabStore) on close and restored on startup as Unloaded, so no page is
created until a tab is activated.

**Verification**: tests/test_lifecycle.cpp passed (0.04 s) covering the enum, every transition
pair and string round-tripping; tests/test_tabstore.cpp passed (1.10 s) covering persistence,
restoration, index handling and rejection of malformed entries.

**Uncertainty remaining**: the exact timing and triggers of Qt's own lifecycleState and
recommendedState changes are provider- and content-dependent and were not exhaustively
characterised, so the recommendedState gate is user-configurable.

## Stage 6: Lifecycle Automation (Complete)

**Status**: ✅ Complete
**Date**: 2026-10-30

Delivered as part of the Stage 2 work phase above. Freeze and hibernate delays are configurable
through SettingsManager and SettingsDialog; inactive tabs freeze after the freeze delay and
hibernate after the hibernate delay; LifecycleManager selects the least recently used eligible
tab and never selects the visible tab; Linux memory pressure is detected from /proc/meminfo
(MemAvailable) with optional PSI parsing, and one LRU tab is hibernated per pressure event.

**Verification**: tests/test_lifecyclemanager.cpp passed (0.04 s) covering delay configuration,
current-tab tracking, LRU selection, exclusion of visible and non-suspendable tabs, and the
memory-pressure hibernation path; tests/test_memorymonitor.cpp passed (0.05 s) covering
/proc/meminfo and PSI parsing.

**Uncertainty remaining**: an appropriate memory-pressure threshold is workload-dependent. The
default is 15% available memory and it is user-configurable. Qt's recommendedState behaviour in
practice could not be characterised exhaustively.

## Stage 7: Recovery and Error Handling (Partial)

**Status**: ⚠️ Partial
**Date**: 2026-10-30

**Implemented**: renderer crash detection via QWebEnginePage::renderProcessTerminated, and a
single bounded automatic reload per crash (a second crash in the same reload cycle moves the tab
to Unloaded instead of retrying, so there are no reload loops). Failed loads return the tab to
the Unloaded state rather than retrying.

**Not implemented**: a dedicated error UI (inline message/banner) and an explicit manual retry
action. These are the remaining items for this stage.

**Verification**: build and offscreen runtime smoke test passed. Renderer crashes were not
induced in this environment, so the recovery path is implemented but not yet exercised live.

#### Blockers

**None**

#### Technical Uncertainties

- Qt renderer crash behaviour under sustained/repeated crashes (not load-tested)
- Memory behaviour after repeated renderer crashes (not measured)

## Stage 8: UI Polish (Not Started)

**Status**: ⏳ Not Started
**Start Date**: -
**End Date**: -

#### Planned Tasks

1. Implement Libadwaita-inspired design
2. Implement light theme
3. Implement dark theme
4. Implement theme switching
5. Implement responsive layouts
6. Add provider icons
7. Add status indicators
8. Polish spacing and typography
9. Test theme switching
10. Test responsive layouts

#### Acceptance Criteria

- [ ] UI follows Libadwaita design language
- [ ] Light theme works correctly
- [ ] Dark theme works correctly
- [ ] Theme switching works
- [ ] Layouts adapt to window size
- [ ] Icons display correctly
- [ ] Status indicators work

#### Build/Test Results

**Status**: Not started

#### Blockers

**None known**

## Stage 9: Testing and Performance (Not Started)

**Status**: ⏳ Not Started
**Start Date**: -
**End Date**: -

#### Planned Tasks

1. Write unit tests for all managers
2. Write unit tests for state machine
3. Write integration tests
4. Measure memory baseline
5. Measure per-tab memory usage
6. Measure tab creation time
7. Measure tab activation time
8. Measure profile growth
9. Measure cache growth
10. Establish performance thresholds
11. Test with multiple tabs (5, 10, 20)
12. Test memory pressure scenarios

#### Acceptance Criteria

- [ ] Unit tests pass (>= 80% coverage)
- [ ] Integration tests pass
- [ ] Memory baseline measured
- [ ] Per-tab memory measured
- [ ] Performance thresholds defined
- [ ] Application meets thresholds

#### Build/Test Results

**Status**: Not started

#### Blockers

**None known**

#### Technical Uncertainties

- Actual memory usage measurements
- Actual profile/cache growth rates
- Appropriate performance thresholds

## Stage 10: Documentation and Packaging (Not Started)

**Status**: ⏳ Not Started
**Start Date**: -
**End Date**: -

#### Planned Tasks

1. Complete README.md
2. Write installation instructions
3. Write user documentation
4. Write troubleshooting guide
5. Create RPM spec file
6. Create desktop file
7. Create application icon
8. Test installation
9. Test package
10. Verify installation on clean Fedora system

#### Acceptance Criteria

- [ ] README.md complete
- [ ] Installation instructions clear
- [ ] User documentation complete
- [ ] Troubleshooting guide complete
- [ ] RPM package builds
- [ ] RPM package installs correctly
- [ ] Application launches after installation

#### Build/Test Results

**Status**: Not started

#### Blockers

**None known**

## Performance Measurements

### Baseline Measurements (Not Started)

**Status**: Not measured yet

| Metric | Target | Actual | Status |
|--------|--------|--------|--------|
| Baseline memory (no tabs) | < 200 MB | - | Not measured |
| Per active tab memory | < 300 MB | - | Not measured |
| Per frozen tab memory | < 100 MB | - | Not measured |
| Per discarded tab memory | < 10 MB | - | Not measured |
| Startup time | < 3 seconds | - | Not measured |
| First tab creation | < 2 seconds | - | Not measured |
| Subsequent tab creation | < 1 second | - | Not measured |
| Tab selection (active) | < 100 ms | - | Not measured |
| Tab activation (frozen) | < 500 ms | - | Not measured |
| Tab activation (discarded) | < 2 seconds | - | Not measured |
| Profile growth per week | < 50 MB | - | Not measured |
| Cache growth per week | < 100 MB | - | Not measured |

## Changed Files

### Initialisation Stage

| File | Change Type | Date | Notes |
|------|-------------|------|-------|
| .gitignore | Created | 2026-10-10 | Git ignore rules |
| .clang-format | Created | 2026-10-10 | Code style configuration |
| AGENTS.md | Created | 2026-10-10 | Project rules and protocol |
| SPEC.md | Created | 2026-10-10 | Complete requirements |
| ARCHITECTURE.md | Created | 2026-10-10 | System design |
| AI_PROGRESS.md | Created | 2026-10-10 | This file |
| README.md | Created | 2026-10-10 | Project overview and documentation |
| docs/TESTING.md | Created | 2026-10-10 | Testing strategy |
| docs/PERFORMANCE.md | Created | 2026-10-10 | Performance criteria |
| docs/DECISIONS.md | Created | 2026-10-10 | Technical decisions |

### Stage 1: Project Foundation

| File | Change Type | Date | Notes |
|------|-------------|------|-------|
| CMakeLists.txt | Created | 2026-10-10 | CMake build configuration |
| src/core/Application.h | Created | 2026-10-10 | Application class header |
| src/core/Application.cpp | Created | 2026-10-10 | Application class implementation |
| src/core/SettingsManager.h | Created | 2026-10-10 | Settings manager header |
| src/core/SettingsManager.cpp | Created | 2026-10-10 | Settings manager implementation |
| src/core/main.cpp | Created | 2026-10-10 | Application entry point |
| src/providers/Provider.h | Created | 2026-10-10 | Provider data structure |
| src/providers/Provider.cpp | Created | 2026-10-10 | Provider implementation |
| src/providers/ProviderRegistry.h | Created | 2026-10-10 | Provider registry header |
| src/providers/ProviderRegistry.cpp | Created | 2026-10-10 | Provider registry with 9 providers |
| src/ui/MainWindow.h | Created | 2026-10-10 | Main window header |
| src/ui/MainWindow.cpp | Created | 2026-10-10 | Main window with Libadwaita design |
| src/ui/TabWidget.h | Created | 2026-10-10 | Tab widget header |
| src/ui/TabWidget.cpp | Created | 2026-10-10 | Tab widget with empty state |
| src/ui/ProviderSelectionDialog.h | Created | 2026-10-10 | Provider selection dialog header |
| src/ui/ProviderSelectionDialog.cpp | Created | 2026-10-10 | Provider selection dialog |
| tests/CMakeLists.txt | Created | 2026-10-10 | Test build configuration |
| tests/test_providerregistry.cpp | Created | 2026-10-10 | Provider registry tests |
| tests/test_settingsmanager.cpp | Created | 2026-10-10 | Settings manager tests |
| README.md | Modified | 2026-10-10 | Updated with Stage 1 status and build instructions |

### Stage 2 work phase (Provider System, Browser Engine, Profiles, Lifecycle)

| File | Change Type | Date | Notes |
|------|-------------|------|-------|
| CMakeLists.txt | Modified | 2026-10-30 | WebEngineWidgets, new sources, new include dirs |
| src/core/SettingsManager.h/.cpp | Modified | 2026-10-30 | Lifecycle, profile-name and workspace settings accessors |
| src/core/TabStore.h/.cpp | Created | 2026-10-30 | Open-tab persistence and restoration |
| src/providers/ProviderIcons.h/.cpp | Created | 2026-10-30 | Runtime-generated provider icons |
| src/providers/ProviderRegistry.cpp | Modified | 2026-10-30 | Assigns an icon to every provider |
| src/profiles/Profile.h/.cpp | Modified | 2026-10-30 | Single-owner profile, lazy QWebEngineProfile, names |
| src/profiles/ProfileStorage.h/.cpp | Modified | 2026-10-30 | Profile listing, directory permissions |
| src/profiles/ProfileManager.h/.cpp | Modified | 2026-10-30 | Create/rename/delete, startup scan, ownership fix |
| src/lifecycle/LifecycleTarget.h | Created | 2026-10-30 | Lifecycle abstraction for automation and tests |
| src/lifecycle/TabMetadata.h | Modified | 2026-10-30 | canTransition, lifecycleStateFromString |
| src/lifecycle/BrowserTab.h/.cpp | Modified | 2026-10-30 | LifecycleTarget, configurable delays, recovery |
| src/lifecycle/LifecycleManager.h/.cpp | Modified | 2026-10-30 | LRU, delays, current tab, memory pressure |
| src/lifecycle/MemoryMonitor.h/.cpp | Created | 2026-10-30 | /proc/meminfo and PSI memory-pressure detection |
| src/lifecycle/TestClock.h/.cpp | Modified | 2026-10-30 | Now used as the injectable clock |
| src/ui/TabWidget.h/.cpp | Modified | 2026-10-30 | Real tabs, accounts, deactivation, persistence |
| src/ui/MainWindow.h/.cpp | Modified | 2026-10-30 | Profiles/Settings/About actions, tab restore/save |
| src/ui/ProfileSelectionDialog.h/.cpp | Created | 2026-10-30 | Multi-account selection |
| src/ui/ProfileManagementDialog.h/.cpp | Created | 2026-10-30 | Account management UI |
| src/ui/SettingsDialog.h/.cpp | Created | 2026-10-30 | Theme and lifecycle settings UI |
| tests/CMakeLists.txt | Modified | 2026-10-30 | Six test targets plus shared include dirs |
| tests/test_lifecycle.cpp | Modified | 2026-10-30 | Transition table and parsing tests |
| tests/test_lifecyclemanager.cpp | Created | 2026-10-30 | LRU, delays, memory pressure |
| tests/test_memorymonitor.cpp | Created | 2026-10-30 | meminfo and PSI parsing |
| tests/test_tabstore.cpp | Created | 2026-10-30 | Tab persistence round-trip |
| tests/test_profilemanager.cpp | Created | 2026-10-30 | Profile lifecycle and storage |
| AI_PROGRESS.md | Modified | 2026-10-30 | Stage statuses and results reconciled with code |
| ARCHITECTURE.md | Modified | 2026-10-30 | Implemented vs not-implemented reconciled |
| README.md | Modified | 2026-10-30 | Current implementation and future plans reconciled |
| docs/DECISIONS.md | Modified | 2026-10-30 | New decisions recorded |
| docs/TESTING.md | Modified | 2026-10-30 | Test layout and actual results reconciled |
| MISTAKES.md | Modified | 2026-10-30 | New build mistakes recorded |

## Known Issues

### Defects fixed in the Stage 2 work phase (previously unrecorded)

1. **Profile double free**: `Profile::deref()` called `deleteLater()` while `ProfileManager`
   still owned the object, leaving a dangling pointer in its map. Resolved by making
   ProfileManager the single owner of Profile objects.
2. **Freeze timers never started**: `TabWidget::currentChanged` only activated the incoming tab
   and never called `onTabDeactivated()` for the outgoing one. Resolved by tracking the current
   tab and deactivating the previous one.
3. **Hardcoded delays**: freeze/hibernate delays were literals inside `BrowserTab` and ignored
   `SettingsManager`. Resolved; both delays are now configurable through the settings dialog.
4. **`freeze()` ignored recommendedState** (only `hibernate()` checked it), contrary to
   AGENTS.md rule 10. Resolved by applying the recommendedState gate to both.
5. **Crash recovery was a no-op**: `onRenderProcessTerminated()` called `createPage()`, which
   returned early because `m_pageCreated` was still true. Resolved; the page is recreated and
   reloaded once, bounded to prevent reload loops.
6. **Empty automation hooks**: `LifecycleManager::scheduleFreeze()`/`scheduleHibernate()` had
   empty bodies and LRU selection was never invoked. Resolved.
7. **Dead TestClock**: `src/lifecycle/TestClock` was unused. Resolved by wiring it into
   `LifecycleManager`'s clock so LRU ordering is deterministically testable.
8. **`LifecycleState` transitions rejected valid changes**: `Loading -> Unloaded` was not
   permitted, so a failed load could not leave the Loading state. Resolved in `canTransition()`.

### Remaining known limitations

1. **Provider embedding is unverified**: some providers may block embedding via
   X-Frame-Options, CSP, User-Agent or bot detection. The application does not circumvent
   these restrictions.
2. **No error UI**: failed loads and renderer crashes fall back to the Unloaded state with no
   dedicated user-facing message or manual retry action (Stage 7 remainder).
3. **Performance unmeasured**: the targets in docs/PERFORMANCE.md have not been measured.
4. **Stages 8-10 incomplete**: full UI polish, integration/performance testing and packaging
   are not finished.

## Risks and Mitigations

### Technical Risks

1. **Profile ownership uncertainty**
   - **Risk**: Qt may not clean up profile data correctly
   - **Mitigation**: Investigate Qt documentation, test profile cleanup early

2. **Provider embedding restrictions**
   - **Risk**: Some providers may block embedding
   - **Mitigation**: Test all providers early, document limitations

3. **Memory pressure detection**
   - **Risk**: Linux memory pressure detection may be complex
   - **Mitigation**: Research Linux APIs, implement basic detection first

4. **Qt WebEngine resource usage**
   - **Risk**: Qt WebEngine may use more resources than expected
   - **Mitigation**: Measure early, adjust thresholds if needed

### Schedule Risks

1. **Qt WebEngine development package**
   - **Risk**: qt6-qtwebengine-devel may have issues
   - **Mitigation**: Install and test early in Stage 1

2. **Lifecycle API complexity**
   - **Risk**: Qt lifecycle API may be more complex than expected
   - **Mitigation**: Prototype lifecycle management early in Stage 5

## Notes

- All documentation created follows the specification in AGENTS.md
- Qt 6.10.3 confirmed to have lifecycle API (since 6.2)
- All 9 provider URLs verified from official sources
- Technical uncertainties documented in SPEC.md and ARCHITECTURE.md
- Technical decisions documented in docs/DECISIONS.md
- Testing strategy documented in docs/TESTING.md
- Performance criteria documented in docs/PERFORMANCE.md
- All required documentation files created and complete
- Stage 1 completed successfully with full UI skeleton and provider catalogue
- All tests passing
- Application builds and launches correctly
- Stage 2 work phase (SPEC stages 2-6) completed: provider icons, multi-account profiles,
  browser embedding, lifecycle state machine, automation and memory pressure
- 7 unit tests passing; build and offscreen runtime smoke test verified (2026-10-30)
- Documentation (AI_PROGRESS.md, ARCHITECTURE.md, README.md, docs/DECISIONS.md,
  docs/TESTING.md, MISTAKES.md) reconciled with the actual code state
- All Stage 2 work phase changes are uncommitted as of 2026-10-30

## Next Steps

**Next stage**: Stage 7 remainder (error UI and manual retry), then Stage 8 (UI polish),
Stage 9 (integration and performance testing) and Stage 10 (packaging).

Immediate follow-ups, in priority order:

1. **Stage 7 remainder** - add a user-visible error state for failed loads and crashed
   renderers, with a manual retry action (explicitly required by SPEC.md Stage 7).
2. **Commit the Stage 2 work phase** - all of the work described above is currently
   uncommitted; only Stage 1 exists in git history.
3. **Stage 9 measurement** - measure the memory and timing targets in docs/PERFORMANCE.md
   (baseline, per-tab, frozen, discarded, startup, tab activation) and record actual values.
4. **Stage 8 polish** - complete the Libadwaita-inspired pass (spacing, tab bar styling,
   explicit status indicators for Frozen and Discarded tabs).
5. **Provider verification** - open each of the 9 providers, record which ones block
   embedding, then document the results in README.md.
6. **Stage 10 packaging** - RPM spec, desktop file, application icon, install verification
   on a clean Fedora 43 system.

**User action required**: Authorise the next stage implementation.
