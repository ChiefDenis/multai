# Technical Decisions

## Document Information

- **Purpose**: Track technical decisions and their rationale throughout the project
- **Status**: Active
- **Last Updated**: 2026-11-12
- **Related Documents**: SPEC.md, ARCHITECTURE.md, AGENTS.md

## Overview

This document records significant technical decisions made during the development of Multai. Each decision includes the context, alternatives considered, the decision made, and the rationale.

## Decision Template

```
### Decision #: [Title]

**Date**: YYYY-MM-DD
**Status**: [Proposed/Accepted/Rejected/Deprecated]
**Context**: [Background and problem statement]
**Alternatives Considered**:
- Alternative 1: [Description]
- Alternative 2: [Description]
- Alternative 3: [Description]

**Decision**: [Selected alternative]

**Rationale**:
- [Reason 1]
- [Reason 2]
- [Reason 3]

**Consequences**:
- Positive: [Consequence 1]
- Positive: [Consequence 2]
- Negative: [Consequence 1]
- Negative: [Consequence 2]

**Related Decisions**: [Decision #1, Decision #2]
**Related Issues**: [Issue reference if applicable]
```

## Decisions

### Decision 001: Qt 6.10.3 as Minimum Version

**Date**: 2026-10-10
**Status**: Accepted
**Context**: Need to select minimum Qt version for the project. Fedora 43 has Qt 6.10.3 available. Qt WebEngine lifecycle API (lifecycleState, recommendedState) was introduced in Qt 6.2.

**Alternatives Considered**:
- Qt 6.2: Minimum version with lifecycle API, but not available on Fedora 43
- Qt 6.5: Older version, may not have all features
- Qt 6.10.3: Version available on Fedora 43, fully featured
- Qt 6.11+: Future version, not yet available on Fedora 43

**Decision**: Use Qt 6.10.3 as minimum supported version

**Rationale**:
- Qt 6.10.3 is the version available on Fedora 43 (target platform)
- Lifecycle API is fully mature in 6.10.3
- Provides all required features for browser lifecycle management
- Widely available and stable

**Consequences**:
- Positive: Matches Fedora 43 system packages
- Positive: Full lifecycle API support
- Positive: Stable and well-tested
- Negative: Won't run on older distributions with older Qt
- Negative: Requires users to have Qt 6.10.3 or later

**Related Decisions**: None
**Related Issues**: None

---

### Decision 002: Qt Widgets over Qt Quick

**Date**: 2026-10-10
**Status**: Accepted
**Context**: Need to choose between Qt Widgets (traditional) and Qt Quick (QML) for UI implementation. The specification requires Qt Widgets.

**Alternatives Considered**:
- Qt Widgets: Traditional C++ UI framework, mature, native look
- Qt Quick (QML): Modern declarative UI, JavaScript-based, more flexible
- Hybrid: Mix of both widgets and quick

**Decision**: Use Qt Widgets exclusively

**Rationale**:
- Specified in requirements
- Better integration with traditional desktop applications
- Native KDE Plasma look and feel
- Pure C++ implementation (no JavaScript runtime)
- More familiar to C++ developers
- Better performance for traditional desktop UIs

**Consequences**:
- Positive: Native desktop integration
- Positive: Pure C++ codebase
- Positive: No JavaScript runtime
- Negative: Less modern than Qt Quick
- Negative: Harder to create custom animations
- Negative: More verbose UI code

**Related Decisions**: None
**Related Issues**: None

---

### Decision 003: CMake Build System

**Date**: 2026-10-10
**Status**: Accepted
**Context**: Need to choose a build system for the C++ project. CMake is the de facto standard for C++ projects and Qt integration.

**Alternatives Considered**:
- CMake: Industry standard, excellent Qt support, cross-platform
- QMake: Qt's original build system, being deprecated
- Meson: Modern, fast, but less Qt integration
- Make: Too low-level, hard to maintain

**Decision**: Use CMake

**Rationale**:
- Industry standard for C++ projects
- Excellent Qt integration (CMake 3.16+)
- Good IDE support (Qt Creator, VS Code, CLion)
- Cross-platform (though we target Fedora only)
- Well-documented
- Large community and ecosystem

**Consequences**:
- Positive: Industry standard
- Positive: Excellent Qt support
- Positive: Good tooling
- Negative: Steeper learning curve than QMake
- Negative: Verbose configuration files

**Related Decisions**: None
**Related Issues**: None

---

### Decision 004: QSettings for Configuration

**Date**: 2026-10-10
**Status**: Accepted
**Context**: Need to store user preferences and application configuration. Options include QSettings, JSON files, INI files, or SQLite.

**Alternatives Considered**:
- QSettings: Qt's built-in settings system, platform-native format
- JSON files: Modern, human-readable, but requires custom parsing
- INI files: Simple, human-readable, but requires custom parsing
- SQLite: Overkill for simple key-value settings

**Decision**: Use QSettings for simple preferences

**Rationale**:
- Built into Qt, no additional dependencies
- Platform-native storage (registry on Windows, plist on macOS, INI on Linux)
- Simple API for key-value access
- Automatic type handling
- Thread-safe

**Consequences**:
- Positive: Built-in, no dependencies
- Positive: Platform-native
- Positive: Simple API
- Negative: Limited to key-value pairs
- Negative: Not ideal for complex structured data

**Related Decisions**: Decision 005 (SQLite for structured data)
**Related Issues**: None

---

### Decision 005: SQLite for Structured Data

**Date**: 2026-10-10
**Status**: Accepted (Conditional)
**Context**: Need to store structured data like tab metadata, navigation history, and workspace configuration. QSettings is insufficient for complex queries.

**Alternatives Considered**:
- SQLite: Embedded SQL database, no server required
- JSON files: Human-readable, but requires custom query logic
- XML files: Structured, but verbose and complex
- Custom binary format: Fast, but complex to implement

**Decision**: Use SQLite for structured workspace persistence (if required)

**Rationale**:
- Embedded, no server required
- SQL queries for complex data retrieval
- ACID transactions for data integrity
- Mature and stable
- Qt has excellent SQLite support (QtSql)
- Lightweight single-file database

**Consequences**:
- Positive: SQL queries for complex data
- Positive: ACID transactions
- Positive: Embedded, no server
- Negative: Additional dependency (though SQLite is very common)
- Negative: SQL complexity for simple queries
- Negative: Need to manage schema migrations

**Related Decisions**: Decision 004 (QSettings for simple preferences)
**Related Issues**: None

---

### Decision 006: Single Process Architecture

**Date**: 2026-10-10
**Status**: Accepted
**Context**: Need to decide between single-process and multi-process architecture. Qt WebEngine spawns renderer processes, but the application itself could be single or multi-process.

**Alternatives Considered**:
- Single process: All application logic in one process, renderer processes separate
- Multi process: Application logic split across multiple processes

**Decision**: Single process architecture

**Rationale**:
- Simpler architecture
- Easier debugging
- Qt WebEngine already spawns renderer processes
- Inter-process communication adds complexity
- No clear benefit to multi-process for this use case

**Consequences**:
- Positive: Simpler architecture
- Positive: Easier debugging
- Positive: No IPC complexity
- Negative: Crash in main process takes down entire app
- Negative: Renderer crashes handled by Qt anyway

**Related Decisions**: None
**Related Issues**: None

---

### Decision 007: Libadwaita-Inspired Design with Qt Widgets

**Date**: 2026-10-10
**Status**: Accepted
**Context**: Specification requires Libadwaita-inspired visual language but implemented with Qt Widgets. Need to determine how to achieve this without using GTK/libadwaita.

**Alternatives Considered**:
- Use GTK/libadwaita directly: Violates specification (no GTK)
- Use Qt Quick with libadwaita style: Violates specification (Qt Widgets only)
- Custom Qt Widgets styling: Time-consuming but compliant
- Qt Style Sheets (QSS): Simple but limited
- Custom painting and widgets: Most flexible but most work

**Decision**: Implement Libadwaita-inspired design using Qt Widgets with custom styling

**Rationale**:
- Complies with specification (Qt Widgets only)
- Qt Style Sheets provide sufficient styling capabilities
- Can achieve rounded corners, spacing, colours through QSS
- Custom widgets for specific components if needed
- Libadwaita is a design language, not a library

**Consequences**:
- Positive: Complies with specification
- Positive: No GTK dependency
- Positive: Qt Style Sheets are powerful enough
- Negative: Requires custom styling work
- Negative: May not perfectly match libadwaita
- Negative: Maintenance burden for custom styles

**Related Decisions**: Decision 002 (Qt Widgets)
**Related Issues**: None

---

### Decision 008: Profile Storage in User Config Directory

**Date**: 2026-10-10
**Status**: Accepted
**Context**: Need to decide where to store browser profiles. Options include user config directory, user data directory, application directory, or custom location.

**Alternatives Considered**:
- ~/.config/ai-workspace/profiles/: Standard config location
- ~/.local/share/ai-workspace/profiles/: Standard data location
- ~/.ai-workspace/profiles/: Custom home directory
- /opt/ai-workspace/profiles/: System-wide (not suitable for user data)

**Decision**: Store profiles in ~/.config/ai-workspace/profiles/

**Rationale**:
- Follows XDG Base Directory Specification
- Standard location for application configuration
- User-specific (no multi-user conflicts)
- Easy to backup and migrate
- Qt WebEngine supports custom profile paths

**Consequences**:
- Positive: Follows standards
- Positive: User-specific
- Positive: Easy to backup
- Negative: Not in XDG data directory (profiles are data, not config)
- Negative: May grow large (config dirs typically small)

**Related Decisions**: None
**Related Issues**: None

---

### Decision 009: Tab State Machine Separate from Qt Lifecycle State

**Date**: 2026-10-10
**Status**: Accepted
**Context**: Qt WebEngine has its own lifecycleState (Active, Frozen, Discarded). The application needs to track additional states (Unloaded, Loading, Closing). Need to decide whether to use Qt's states directly or maintain separate state machine.

**Alternatives Considered**:
- Use Qt's lifecycleState directly: Simpler, but limited to 3 states
- Separate state machine that maps to Qt states: More complex, but more flexible
- Hybrid: Use Qt states where possible, extend where needed

**Decision**: Maintain separate tab state machine, synchronize with Qt lifecycleState

**Rationale**:
- Application needs 6 states (Unloaded, Loading, Active, Frozen, Discarded, Closing)
- Qt only provides 3 states (Active, Frozen, Discarded)
- Additional states needed for lazy loading and cleanup
- Clear separation between application logic and Qt implementation
- Easier to test application logic independently

**Consequences**:
- Positive: More expressive state machine
- Positive: Clear separation of concerns
- Positive: Easier to test
- Negative: Additional complexity
- Negative: Need to synchronize states
- Negative: Potential for state mismatch

**Related Decisions**: None
**Related Issues**: None

---

### Decision 010: Lazy Page Creation

**Date**: 2026-10-10
**Status**: Accepted
**Context**: Need to decide when to create browser pages. Options include creating all pages at startup, creating on tab creation, or creating on tab selection.

**Alternatives Considered**:
- Create all pages at startup: Fastest tab switching, but high memory usage
- Create on tab creation: Balanced, but still creates pages for inactive tabs
- Create on tab selection (lazy): Lowest memory usage, but slower first activation

**Decision**: Lazy page creation (create only when tab is selected)

**Rationale**:
- Minimises memory usage
- Aligns with resource-conscious design goals
- Acceptable trade-off: slower first activation (but < 2 seconds target)
- Qt WebEngine process creation is expensive, avoid where possible
- Users may not use all tabs they create

**Consequences**:
- Positive: Minimises memory usage
- Positive: Faster application startup
- Positive: Resource-conscious
- Negative: Slower first tab activation
- Negative: May surprise users expecting instant loading

**Related Decisions**: None
**Related Issues**: None

---

### Decision 011: Respect Qt's recommendedState by Default

**Date**: 2026-10-10
**Status**: Accepted
**Context**: Qt WebEngine provides recommendedState which suggests the safest lifecycle state. Need to decide whether to always respect it or allow overrides.

**Alternatives Considered**:
- Always respect recommendedState: Safest, but may freeze tabs user wants active
- Ignore recommendedState: More control, but may cause side effects
- Respect by default, allow documented overrides: Balanced

**Decision**: Respect recommendedState by default, allow user-configurable overrides with documented side effects

**Rationale**:
- recommendedState is based on page activity (audio, form input, etc.)
- Ignoring it may cause data loss or poor user experience
- Power users may want to override for specific use cases
- Documentation of side effects allows informed decisions
- Balance between safety and flexibility

**Consequences**:
- Positive: Safe default behaviour
- Positive: Prevents data loss
- Positive: Flexible for power users
- Negative: Additional UI complexity
- Negative: Users may not understand side effects
- Negative: More configuration options

**Related Decisions**: None
**Related Issues**: None

---

### Decision 012: LRU Selection for Resource Allocation

**Date**: 2026-10-10
**Status**: Accepted
**Context**: When resources are limited (memory pressure), need to decide which tabs to freeze/hibernate. Options include LRU, random, oldest, or user-specified priority.

**Alternatives Considered**:
- Least Recently Used (LRU): Freezes tabs user hasn't used recently
- Random: Unpredictable, may freeze active tabs
- Oldest: May freeze tabs user wants to keep
- User-specified priority: Additional UI complexity

**Decision**: Use Least Recently Used (LRU) selection

**Rationale**:
- Simple to implement
- Intuitive behaviour (likely what user expects)
- No additional UI complexity
- Effective for most use cases
- User can manually pin important tabs if needed (future feature)

**Consequences**:
- Positive: Simple implementation
- Positive: Intuitive behaviour
- Positive: No UI complexity
- Negative: May freeze tabs user wants to keep
- Negative: No user control over priority

**Related Decisions**: None
**Related Issues**: None

---

### Decision 013: QSettings INI Format for Portability

**Date**: 2026-10-10
**Status**: Accepted
**Context**: QSettings can use platform-native formats (registry on Windows, plist on macOS) or a consistent INI format across platforms. Need to decide which to use.

**Alternatives Considered**:
- Platform-native format: Better integration, but harder to migrate
- INI format: Consistent across platforms, human-readable, easier to debug

**Decision**: Use INI format for consistency and portability

**Rationale**:
- Consistent across platforms (though we target Fedora only)
- Human-readable, easier to debug
- Easier to migrate and backup
- Users can edit manually if needed
- No dependency on platform-specific APIs

**Consequences**:
- Positive: Consistent format
- Positive: Human-readable
- Positive: Easy to debug
- Positive: Easy to migrate
- Negative: Not platform-native
- Negative: May be slightly slower than native formats

**Related Decisions**: Decision 004 (QSettings)
**Related Issues**: None

---

## Decisions Resolved (Stage 2 Work Phase, 2026-10-30)

### Decision 014: Memory Pressure Detection via /proc/meminfo with Optional PSI

**Date**: 2026-10-30
**Status**: Accepted
**Context**: Hibernation must also be triggered by system memory pressure, not only by elapsed time. Linux exposes several mechanisms and not all are guaranteed to be present or enabled.

**Alternatives Considered**:
- /proc/meminfo MemAvailable: Always present on Linux; simple text parsing
- /proc/pressure/memory (PSI): More accurate about stalls, but only present when CONFIG_PSI is enabled
- systemd resource control notifications: Requires D-Bus integration and a systemd-specific design
- cgroups v2 memory controller: Requires the application to be placed in a cgroup by the user

**Decision**: The primary signal is MemAvailable from /proc/meminfo compared against a configurable percentage threshold (default 15%). PSI `some avg10` parsing is implemented and used when readable, but is not required.

**Rationale**:
- /proc/meminfo is universal on the target platform and cheap to read
- A percentage of total is stable across machines with different RAM sizes
- PSI parsing is additive, so no hard dependency is created
- A configurable threshold avoids hardcoding assumptions about workload or machine size

**Consequences**:
- Positive: Works on any Fedora 43 system without extra privileges
- Positive: The threshold is user-configurable in the settings dialog
- Negative: MemAvailable is a coarse signal; it cannot prevent allocation failures, only react
- Negative: The 15 s polling interval introduces detection latency

**Related Decisions**: Decision 012 (LRU selection)
**Related Issues**: Stage 6

---

### Decision 015: Profile Cleanup by Recursive Directory Removal

**Date**: 2026-10-30
**Status**: Accepted
**Context**: Deleting an account must remove its persistent cookies, storage and cache without corrupting Qt WebEngine state.

**Alternatives Considered**:
- Delete the whole profile directory recursively: Simple and complete
- Let Qt clean up: Qt does not delete persistent profile directories
- Delete only cache/storage subdirectories: Selective, but risks orphaned data and is harder to reason about

**Decision**: Deleting an account removes the entire profile directory recursively, and deletion is refused while any tab still references the profile (`refCount() > 0`).

**Rationale**:
- A profile directory contains only that account's isolated data, so wholesale removal is safe and complete
- The reference-count guard prevents deleting a profile in use by an open tab
- The profile display name is removed from QSettings at the same time, leaving nothing behind

**Consequences**:
- Positive: No orphaned profile data after account deletion
- Positive: No dangling QWebEngineProfile while tabs are open
- Negative: Deletion is irreversible, so the UI asks for confirmation first
- Negative: The profile must be closed (refCount 0) before it can be deleted

**Related Decisions**: Decision 008 (profile storage location), Decision 016 (profile ownership)
**Related Issues**: Stage 3

---

### Decision 016: ProfileManager is the Single Owner of Profile Objects

**Date**: 2026-10-30
**Status**: Accepted
**Context**: The in-flight implementation had `Profile::deref()` call `deleteLater()` at reference count zero, while `ProfileManager` still held the pointer in its map and also ran `qDeleteAll` in its destructor: a dangling-pointer and double-free hazard.

**Alternatives Considered**:
- Shared ownership (QSharedPointer): Correct, but invasive for an object that already has a QObject parent
- Profile self-deletes at refCount zero: The original design, which leaves the manager with a stale pointer
- ProfileManager owns exclusively, with refCount only gating operations: simple and explicit

**Decision**: ProfileManager owns every Profile exclusively. `Profile::deref()` only decrements the counter and never deletes. Deletion happens only through `ProfileManager::deleteProfile()`, which refuses while the reference count is non-zero.

**Rationale**:
- A single owner removes all ambiguity about when a Profile is destroyed
- Children are deleted exactly once, by `deleteProfile` or by the manager destructor
- Keeping `Profile` alive while unused is desirable: cached Profile objects make reopening an account cheap

**Consequences**:
- Positive: No dangling pointers or double frees
- Positive: Profiles stay cached, so account switching is fast
- Negative: Memory for idle Profile objects is retained until deletion or exit
- Negative: QWebEngineProfile destruction now happens inside `Profile::~Profile`

**Related Decisions**: Decision 010 (lazy page creation), Decision 015 (profile cleanup)
**Related Issues**: Fixed defect 1 in AI_PROGRESS.md

---

### Decision 017: Provider Embedding Compatibility Detection

**Date**: 2026-10-30
**Status**: Proposed (still open)
**Context**: Some providers may refuse to load inside an embedded view, using X-Frame-Options, Content Security Policy, User-Agent or bot detection. AGENTS.md forbids circumventing these restrictions.

**Alternatives Considered**:
- Monitor load failures: Simple; `loadFinished(false)` and `renderProcessTerminated` are handled, but a provider returning an error page still reports success
- Inspect response headers: More accurate, but requires a request interceptor and provider-specific parsing
- User-reported: Simplest; leaves the user to notice and report

**Decision**: Not yet decided. Current behaviour is deliberately passive: load failures return the tab to Unloaded and nothing is done to work around a provider that blocks embedding. No circumvention is implemented or planned.

**Rationale**:
- The security constraints in AGENTS.md rule 8 take precedence over making every provider work
- Any detection heuristic must be documented as a limitation rather than worked around

**Consequences**:
- Positive: No security circumvention and no identity spoofing
- Negative: Some providers may be unusable in the application
- Negative: No built-in user-visible explanation of a blocked provider yet

**Related Decisions**: Decision 020 (recommendedState gate)
**Related Issues**: Stage 7 remainder; README known limitations

---

### Decision 018: LifecycleTarget Interface Decouples Automation from Qt WebEngine

**Date**: 2026-10-30
**Status**: Accepted
**Context**: `LifecycleManager` originally stored `BrowserTab*` keys and called `activate()`, `freeze()` and `hibernate()` directly. That forced any test of the automation policy to link Qt WebEngine and, for a real page, to have a display available.

**Alternatives Considered**:
- Keep `BrowserTab*` in LifecycleManager: simple, but untestable without WebEngine
- Template the manager on the tab type: avoids virtual calls, but inflates the header and complicates moc
- Introduce a small abstract `LifecycleTarget` interface: one vtable per tab, and the manager depends only on the interface

**Decision**: Introduce `LifecycleTarget` (state query, suspendability query, activate, deactivate, freeze, hibernate). `BrowserTab` inherits `QWidget` and `LifecycleTarget`. `LifecycleManager` operates only on `LifecycleTarget*`.

**Rationale**:
- The automation policy (delays, LRU, memory pressure) is pure logic and should be testable in isolation
- Dependency inversion means `LifecycleManager` no longer includes Qt WebEngine headers at all
- Tests can use a fake target, so LRU and pressure behaviour are verified without a browser

**Consequences**:
- Positive: `test_lifecyclemanager` runs in 0.04 s with no WebEngine dependency
- Positive: Policy and browser mechanics can evolve independently
- Negative: One extra indirection per lifecycle call (negligible)
- Negative: Signals carrying `LifecycleTarget*` are not queued-connection friendly without a registered metatype

**Related Decisions**: Decision 009 (state machine separate from Qt lifecycle state)
**Related Issues**: Stage 6

---

### Decision 019: Provider Icons Generated at Runtime

**Date**: 2026-10-30
**Status**: Accepted
**Context**: SPEC.md requires an icon per provider. Shipping nine binary assets would add a resource pipeline, licensing questions for third-party logos, and `.qrc` maintenance.

**Alternatives Considered**:
- Bundle official provider logos: best fidelity, but licensing and branding concerns
- Bundle hand-made SVG icons: cleaner licensing, but still binary assets plus a resource pipeline
- Generate rounded colour tiles with the provider initial at runtime: no assets, deterministic

**Decision**: Generate icons at runtime with QPainter: a rounded rectangle filled with a per-provider brand colour, overlaid with the provider's initial. Exposed via `providerIcon(providerId)`.

**Rationale**:
- No third-party logos are redistributed, avoiding branding and licensing issues
- No `.qrc` or asset pipeline is required, and icons scale to any requested size
- The visual language (rounded surfaces, restrained colours) matches the Libadwaita direction
- Colours and initials live in one table, so adding a provider is a one-line change

**Consequences**:
- Positive: Zero binary assets and no licensing ambiguity
- Positive: Icons are generated at multiple sizes for tab bars and lists
- Negative: Icons are generic colour tiles rather than official logos
- Negative: Icons require a QGuiApplication, so they cannot be created before QApplication exists

**Related Decisions**: Decision 002 (Qt Widgets), Decision 007 (Libadwaita-inspired design)
**Related Issues**: Stage 2

---

### Decision 020: recommendedState Gate is User-Configurable

**Date**: 2026-10-30
**Status**: Accepted
**Context**: AGENTS.md rule 10 requires respecting `QWebEnginePage::recommendedState` by default while allowing user-configurable overrides only when side effects are clearly documented. The exact triggers of Qt's `recommendedState` are provider- and content-dependent and were not exhaustively characterised in this environment.

**Alternatives Considered**:
- Always respect recommendedState with no override: safest, but a page that never reports a suspendable state can never be frozen
- Always ignore recommendedState: maximises resource savings, but per Qt documentation risks stopping background audio and discarding unsaved form input
- Respect it by default with a documented override setting: complies with AGENTS.md and keeps the escape hatch explicit

**Decision**: Add the setting `lifecycle/respectRecommendedState`, default true. When true, a tab may only freeze if `recommendedState != Active`, and may only be discarded if `recommendedState == Discarded`. When false, suspension is allowed regardless, and the settings dialog states the side effects in the control tooltip.

**Rationale**:
- Default behaviour complies with AGENTS.md rule 10 and with Qt's documented soft limits
- The override is opt-in, so users must actively accept the documented consequences
- Structural guarantees still hold regardless of the setting: the visible tab and loading tabs are never suspended

**Consequences**:
- Positive: Safe by default; no silent loss of form input or background audio
- Positive: Users can enable aggressive suspension when they accept the trade-off
- Negative: With the gate enabled, tabs whose pages stay Active will never freeze or hibernate
- Negative: The interaction between Qt-recommended state and per-provider behaviour remains unverified

**Related Decisions**: Decision 011 (respect recommendedState by default), Decision 017 (embedding detection)
**Related Issues**: Stage 6

---

### Decision 021: No Separate TabManager Class

**Date**: 2026-10-30
**Status**: Accepted
**Context**: ARCHITECTURE.md originally described a TabManager coordinating tab lifecycle, and SPEC.md Stage 4 listed it. During implementation, tab ownership naturally landed in TabWidget (which owns the QTabWidget and the tab widgets) and policy landed in LifecycleManager.

**Alternatives Considered**:
- Introduce TabManager as a third coordinator: matches the original sketch, but duplicates state already held by TabWidget and LifecycleManager
- Keep ownership in TabWidget and policy in LifecycleManager: fewer moving parts, one owner per concern

**Decision**: Do not introduce a TabManager. TabWidget owns tab instances, activation, close and persistence; LifecycleManager owns delays, LRU selection and memory-pressure policy.

**Rationale**:
- Each concern has exactly one owner, so no state has to be synchronised between two managers
- TabWidget already had to track tab widgets for layout and current-index handling
- The architecture document's TabManager was a sketch, not a requirement, and the deviation is recorded explicitly

**Consequences**:
- Positive: Fewer classes and no duplicated tab registry
- Positive: Policy remains testable through LifecycleTarget without widgets
- Negative: TabWidget is a larger class than the original sketch implied
- Negative: ARCHITECTURE.md needed updating to record the deviation

**Related Decisions**: Decision 018 (LifecycleTarget)
**Related Issues**: Stage 4 deviation recorded in AI_PROGRESS.md

---

### Decision 022: Explicit QPalette Alongside Stylesheets

**Date**: 2026-11-12
**Status**: Accepted
**Context**: Stylesheet-only theming left several surfaces dependent on the system
palette. Dark-mode dialogs, status-bar labels and list alternates inherited light
colours on some Wayland sessions, and `Qt::WA_StyledBackground` was not sufficient for
every composite widget.

**Alternatives Considered**:
- Add `Qt::WA_StyledBackground` to every affected widget: fixes only widgets that opt in
  and still depends on a parent painting behind them
- Style each widget individually: duplicates colours and misses native-drawn parts
- Apply an explicit light/dark `QPalette` in addition to the stylesheet: one authoritative
  colour source for both styled and natively drawn surfaces

**Decision**: `Theme::applyTheme()` sets an explicit `QPalette` for the effective mode
(light/dark/system) in addition to the application stylesheet.

**Rationale**:
- Dialogs, tooltips, disabled text and placeholder text use palette colours that a
  stylesheet does not always override
- A single palette removes dependence on the host desktop palette
- System mode is resolved once through `QStyleHints::colorScheme()` with a palette
  lightness fallback

**Consequences**:
- Positive: Consistent dark and light surfaces across widgets and dialogs
- Positive: Disabled and placeholder states have deliberate contrast
- Negative: Palette and stylesheet colour values must be kept in sync manually

**Related Decisions**: Decision 007 (Libadwaita-Inspired Design), Decision 024 (ThemedDialog)
**Related Issues**: Visual QA dark-surface defects

---

### Decision 023: Painted Application-Owned Toolbar Icons

**Date**: 2026-11-12
**Status**: Accepted
**Context**: Platform standard icons (`SP_ArrowBack`, `SP_BrowserReload`,
`SP_DesktopIcon`) were missing, blurred or low-contrast under some icon themes. The
global button padding also collapsed the icon content box.

**Alternatives Considered**:
- Depend on the system icon theme: no control over availability or contrast
- Ship SVG assets: adds resource/theme plumbing and still needs disabled variants
- Paint icons at runtime for each required size and state: deterministic and testable

**Decision**: Generate Back, Forward, Reload, Open External, folder, tabs, settings and
close icons in `src/ui/ToolbarIcons.cpp` using `QPainter`, with Normal and Disabled
pixmaps for 16/20/24/32/48/64 px and separate light/dark colours.

**Rationale**:
- Icons render identically regardless of the desktop icon theme
- Disabled states have deliberate contrast
- Painted pixmaps can be asserted in tests without external assets

**Consequences**:
- Positive: Reliable, theme-aware toolbar affordances
- Positive: Icon rendering is unit-testable
- Negative: Icon shapes must be maintained in code rather than replaced by designers

**Related Decisions**: Decision 019 (Provider Icons Generated at Runtime)
**Related Issues**: Visual QA blank-toolbar defect

---

### Decision 024: ThemedDialog Paints Its Own Background

**Date**: 2026-11-12
**Status**: Accepted
**Context**: `QDialog` did not paint its stylesheet or palette background on Wayland when
captured through `QWidget::grab()`, so the account and settings dialogs appeared light
even in dark mode.

**Alternatives Considered**:
- Set `Qt::WA_StyledBackground` on each dialog: did not reliably paint the dialog surface
  in the captured window
- Use a frameless custom window: changes window behaviour and decorations
- Derive application dialogs from a small base class that fills the background explicitly

**Decision**: Add `src/ui/ThemedDialog.h`; all application-owned dialogs derive from it and
fill `rect()` with `palette().color(QPalette::Window)` before normal dialog painting.

**Rationale**:
- Guarantees a themed surface for capture and for compositors that do not paint the
  dialog background
- Keeps native dialog decoration and modality behaviour
- One small base class instead of per-dialog workarounds

**Consequences**:
- Positive: Account, provider, profile and settings dialogs are readable in both themes
- Positive: Dialog captures can be asserted in tests
- Negative: Dialogs must derive from `ThemedDialog` rather than `QDialog` directly

**Related Decisions**: Decision 022 (Explicit QPalette)
**Related Issues**: Visual QA dark-dialog defect

---

### Decision 025: State-Asserted Screenshot Harness with WebEngine Compositing

**Date**: 2026-11-12
**Status**: Accepted
**Context**: `QWidget::render()` does not include the composited `QWebEngineView`
surface, and fixed-delay captures could record a `Loading` state instead of the intended
state. Silent captures made visual regressions easy to miss.

**Alternatives Considered**:
- Capture with `render()` only: web content is always blank
- Increase fixed delays: slow and still racy
- Offscreen QPA: does not paint WebEngine content
- Run on a real Wayland/X11 session, composite `QWebEngineView::grab()`, and assert each
  state before writing the image

**Decision**: The visual QA launcher asserts the expected tab count, active tab,
lifecycle state, header visibility, sidebar mode, empty state or exposed dialog before
capture, composites the web view grab into the window image, and fails the capture when
an assertion fails.

**Rationale**:
- Captures represent a verified application state, not a timer guess
- WebEngine content is included in the archive
- Failures are visible instead of producing plausible but wrong screenshots

**Consequences**:
- Positive: 12 reproducible light/dark state captures
- Positive: Harness can gate visual regressions in the future
- Negative: Requires an interactive Wayland or X11 session
- Negative: Live provider content still depends on external networks

**Related Decisions**: Decision 010 (Lazy Page Creation)
**Related Issues**: Visual QA blank-capture and persistent-Loading defects

---

### Decision 026: Provider Display Names as Tab Titles

**Date**: 2026-11-12
**Status**: Accepted
**Context**: Restored tabs showed persisted page titles such as "DeepSeek - Into the
Unknown", and page titles could also collapse into a URL or hostname, producing
inconsistent and unstable tab labels.

**Alternatives Considered**:
- Use live page titles: unstable, provider-controlled and sometimes a URL
- Use metadata page titles on restore: differs from the live title and can be stale
- Use provider display names consistently: stable and predictable

**Decision**: Tab row text is always the provider display name. Page titles continue to be
stored in metadata but are not used for tab row text.

**Rationale**:
- Tabs remain identifiable even when a provider changes its document title
- Restored and new tabs present identically
- Provider identity matches the sidebar and workspace header

**Consequences**:
- Positive: Stable, consistent tab labels
- Positive: No URL or hostname leakage in tab labels
- Negative: A single account cannot distinguish two tabs of the same provider by title;
  lifecycle and error state still differentiate them

**Related Decisions**: Decision 019 (Provider Icons Generated at Runtime)
**Related Issues**: Visual QA inconsistent-tab-title defect

---

### Decision 027: Versioned Session Schema in QSettings

**Date**: 2026-11-13
**Status**: Accepted
**Context**: 0.4.0 restores open tabs across restarts. The previous `TabStore` wrote a
plain list with no schema marker and no validation, so a malformed or future-version
record could restore an invalid provider, an unrelated URL or an unsafe profile reference.

**Alternatives Considered**:
- Store the whole workspace (including page content) in SQLite: heavier and does not
  restore authentication
- Persist only the provider list: loses ordering, profiles and meaningful chat URLs
- Keep the existing flat list without validation: minimal change but brittle
- Versioned QSettings record with provider and URL validation

**Decision**: `TabStore` writes `workspace/sessionVersion = 1`, a bounded list of
records containing `providerId`, `profileId`, `url`, `title` and `lifecycleState`, plus
`workspace/currentTabIndex`. On load, providers are resolved through
`ProviderRegistry`, URLs are checked with `Provider::isUrlAllowed()` and fall back to the
provider chat URL, unknown providers are skipped, and a session written by a newer schema
is left untouched. Sessions are capped at 50 tabs.

**Rationale**:
- A schema marker allows future migrations without guessing record shape
- Provider allowlists prevent a corrupted record from opening an arbitrary origin
- Leaving a future session intact avoids destroying data written by a newer release
- QSettings keeps the existing dependency-free, portable storage path

**Consequences**:
- Positive: Corrupt or hostile session records cannot redirect a tab off-provider
- Positive: Forward compatibility is preserved by refusing to overwrite newer schemas
- Negative: Page DOM, form state and scroll position are not restored
- Negative: A session can contain fewer tabs than were open if providers are removed

**Related Decisions**: Decision 004 (QSettings), Decision 013 (INI format),
Decision 010 (Lazy Page Creation)
**Related Issues**: 0.4.0 persistent session requirement

---

### Decision 028: Restore Tabs Unloaded and Activate Only the Saved Selection

**Date**: 2026-11-13
**Status**: Accepted
**Context**: Restoring every tab eagerly would create one WebEngine page per saved tab,
spiking memory and network traffic at startup and competing with the user's selected tab.

**Alternatives Considered**:
- Recreate all pages during restore: simple but resource-heavy
- Restore tab metadata only and drop the selected tab's URL: loses the workspace
- Create all tabs in `Unloaded` state and activate only the saved index

**Decision**: All restored tabs are added with `LifecycleState::Unloaded` and
`activate = false`. The tab matching `workspace/currentTabIndex` is activated afterwards,
which creates its page and navigates to the validated URL. Inactive restored tabs keep
`page() == nullptr` until selected.

**Rationale**:
- Matches the existing lazy page-creation contract
- Startup cost is proportional to one page, not the whole session
- Lifecycle automation can manage restored tabs exactly like newly opened ones
- Sidebar and header selection stay consistent with the active tab

**Consequences**:
- Positive: Low startup memory and no background reload storm
- Positive: Inactive tabs show `Unloaded` until the user needs them
- Negative: Switching to a restored tab always performs a fresh navigation

**Related Decisions**: Decision 010 (Lazy Page Creation), Decision 012 (LRU)
**Related Issues**: 0.4.0 restore requirement

---

### Decision 029: Horizontal Provider Strip with Painted Buttons

**Date**: 2026-11-13
**Status**: Accepted
**Context**: A vertical nine-row provider list consumed most of the sidebar height and
left little room for Open Tabs. The request was a compact, single-row provider selector.

**Alternatives Considered**:
- Keep the vertical `QListWidget` and shrink rows: still consumes vertical space
- Use a `QComboBox`: hides providers behind a popup and weakens discoverability
- Use a horizontal `QListWidget` in icon mode: workable but more configuration and
  inconsistent painting
- Horizontal `QScrollArea` containing fixed 42x42 painted `QToolButton`s

**Decision**: Providers are shown in one horizontal `QScrollArea` strip of owned painted
buttons. The strip scrolls with the wheel, shows tooltips, marks the selected provider
with a dynamic `selected` property, and mirrors selection in the collapsed rail. Open Tabs
remains a vertical list.

**Rationale**:
- One row of icons preserves vertical space for tabs
- Native horizontal scrolling keeps all nine providers reachable on narrow sidebars
- Painted buttons match the existing application-owned icon rendering
- A 5 px spacing lets exactly five icons fit without partial clipping

**Consequences**:
- Positive: Provider selection is compact and recognisable by icon
- Positive: Collapsed and expanded sidebars stay synchronised
- Negative: Provider names require tooltips when the strip is narrow
- Negative: Wheel-over-strip scrolls horizontally instead of the tab list

**Related Decisions**: Decision 019 (runtime icons), Decision 023 (painted icons)
**Related Issues**: 0.4.0 compact-navigation requirement

---

### Decision 030: Remove the Workspace Navigation Toolbar

**Date**: 2026-11-13
**Status**: Accepted
**Context**: The Back/Forward/Reload/Open External row duplicated provider-site controls,
consumed vertical space and added callbacks and icons that no longer served the
provider-embedding workflow.

**Alternatives Considered**:
- Keep the toolbar hidden by default: leaves dead widgets and ambiguous state
- Keep only Reload: still a partial toolbar with the same layout cost
- Remove the toolbar entirely and rely on provider pages plus the existing menu bar

**Decision**: The workspace header contains only the provider icon and name. All
navigation toolbar widgets, callbacks, member variables and their `ToolbarIcons` entries
are deleted. Existing menu actions and status indicators remain.

**Rationale**:
- Providers render their own navigation and reload affordances
- Removing the row gives the browser content the maximum usable height
- Deleted code is better than disabled code for maintenance and tests

**Consequences**:
- Positive: Cleaner header and more content area
- Positive: No stale callbacks or unused icon code
- Negative: Users lose an in-app reload button; the menu bar and provider UI cover this

**Related Decisions**: Decision 023 (painted toolbar icons)
**Related Issues**: 0.4.0 toolbar-removal requirement
**Superseded in part by**: Decision 032 (the remaining provider header was removed)

---

### Decision 031: Bundle Authentic Provider SVG Logos and Render Them as Theme-Aware Icons

**Date**: 2026-11-13
**Status**: Accepted
**Context**: The painted monogram icons in the provider strip, collapsed rail, Open Tabs
and empty state were placeholders. Users need to recognise the nine AI providers at a
glance, in both light and dark themes, without introducing a new rendering dependency.

**Alternatives Considered**:
- Keep painted monograms: not authentic and easily confused
- Download logos at runtime: adds network activity and failure modes
- Link `Qt6::Svg`: requires the uninstalled `qt6-qtsvg-devel` package for headers
- Bundle SVG files and read them through Qt's existing SVG image-format plugin

**Decision**: Store one authentic SVG per provider under `assets/providers/`, ship them in
a `provider-logos` static resource library, and render them with `QImageReader` through
the qsvg image plugin. Scale each mark aspect-preservingly inside an 8% inset on a
transparent square pixmap. Tint only the monochrome OpenAI, Anthropic and Kimi marks to
the current foreground colour; keep every coloured mark in its original brand colours.
Expose `Provider::icon()` as a live render so theme changes refresh the strip, rail, tab
rows and empty-state grid.

**Rationale**:
- Authentic paths and proportions preserve brand recognition
- The qsvg plugin is already present with the Qt WebEngine runtime
- Theme-aware monochrome marks stay legible without recolouring distinctive artwork
- Bundling avoids runtime downloads and keeps the application self-contained

**Consequences**:
- Positive: Consistent, sharp logos from 16 px to 42 px and above
- Positive: No new package or link dependency
- Positive: Light and dark themes both remain legible
- Negative: Logo assets must be kept with attribution and updated deliberately
- Negative: `Provider::icon()` re-renders instead of returning a cached pixmap

**Related Decisions**: Decision 019 (runtime icons), Decision 023 (painted application
icons)
**Related Issues**: Provider brand-logo requirement

---

### Decision 032: Remove the Remaining Workspace Provider Header

**Date**: 2026-11-13
**Status**: Accepted
**Context**: After the navigation toolbar removal, a 44 px header still displayed the
provider icon and name above the embedded page. It duplicated the sidebar selection and
reduced the browser's usable height.

**Alternatives Considered**:
- Hide only the labels: leaves the container, margins and reserved space
- Keep a slim title row: still duplicates sidebar identity
- Remove the header widget, layout, members, callbacks and test hooks entirely

**Decision**: `BrowserWorkspace` is added directly to the main splitter. No header,
container, layout, separator or reserved top gap remains. The menu bar, sidebar, status
bar, session restoration and lifecycle indicators are unchanged.

**Rationale**:
- The sidebar already identifies the selected provider
- Provider pages render their own titles and navigation
- The embedded page receives the full available workspace area
- Deleting the whole path avoids dead hooks and hidden layout cost

**Consequences**:
- Positive: Maximum browser height and no duplicated identity chrome
- Positive: Fewer widgets, callbacks and stylesheet rules
- Negative: Provider identity above the page is available only in the sidebar and status
  bar, not in a workspace title row

**Related Decisions**: Decision 030 (navigation toolbar removal)
**Related Issues**: Workspace header-removal requirement

---

### Decision 033: Remove Sidebar Branding Header

**Date**: 2026-11-13
**Status**: Accepted
**Context**: The expanded sidebar displayed an application logo and name ("AI Workspace")
at the top, consuming ~70px of vertical space. This branding was redundant with the
window title, taskbar, and application launcher, and pushed the provider strip and
Open Tabs sections down.

**Alternatives Considered**:
- Hide contents only: leaves the empty header container and divider
- Move branding to bottom: still consumes space
- Remove the header widget, divider, and stylesheet rules entirely

**Decision**: Deleted the `sidebarHeader` widget (56px), its `sidebarDivider` (1px),
`m_headerAppIcon`, `m_headerAppLabel` members, and all `#sidebarHeader` stylesheet
rules. The provider section now starts at the top of the expanded sidebar container.

**Rationale**:
- Reclaims ~70px of vertical space for provider strip and Open Tabs
- No functional loss: application identity is in window title, taskbar, launcher
- Simplifies sidebar layout and reduces stylesheet complexity

**Consequences**:
- Positive: More vertical space for providers and tabs
- Positive: Cleaner sidebar layout
- Negative: Application logo no longer visible in expanded sidebar (still in window/taskbar)

**Related Decisions**: Decision 032 (workspace header removal)
**Related Issues**: Stage 1 of 0.4.1 release

---

### Decision 034: Application-Wide Tooltip Palette

**Date**: 2026-11-13
**Status**: Accepted
**Context**: Native Qt tooltips had poor contrast in both themes (black text on black
background in dark mode, light text on light background in light mode). Per-widget
stylesheet fixes were inconsistent and didn't cover native tooltips.

**Alternatives Considered**:
- Per-widget tooltip stylesheets: incomplete, doesn't cover native tooltips
- Custom tooltip widget: unnecessary complexity
- Explicit QPalette ToolTipBase/ToolTipText + QSS QToolTip rule

**Decision**: Added `QPalette::ToolTipBase` and `QPalette::ToolTipText` to
`paletteForMode()` for both light and dark themes. Added `QToolTip` stylesheet rule
with themed background, text, border, padding, and font-size.

**Rationale**:
- Single authoritative source for tooltip appearance
- Works for both native Qt tooltips and custom tooltip widgets
- Theme-aware: light theme uses white surface/dark text, dark theme uses dark surface/light text
- Covers all tooltips: providers, tabs, sidebar controls, settings, account management

**Consequences**:
- Positive: Consistent tooltip contrast across entire application
- Positive: No per-widget hardcoding required
- Positive: Automatically updates on theme change
- Negative: Palette and QSS values must be kept in sync

**Related Decisions**: Decision 022 (Explicit QPalette)
**Related Issues**: Stage 2 of 0.4.1 release

---

### Decision 035: Settings Screen Contrast Audit and Refinement

**Date**: 2026-11-13
**Status**: Accepted
**Context**: Settings dialog had inconsistent contrast across states (hover, focus,
disabled, placeholder) in both themes. Some labels used colours intended for other
contexts.

**Alternatives Considered**:
- Fix individual widgets: scattered changes, easy to miss states
- Full contrast audit with explicit palette/stylesheet for all controls

**Decision**: Added explicit `QCheckBox`, `QRadioButton`, `QDialogButtonBox` styling
to both themes. Defined indicator sizes, checked/unchecked/hover/disabled states with
theme-appropriate colours. Verified placeholder text, focus indicators, disabled
states, and form label visibility.

**Rationale**:
- Systematic coverage of all control states
- WCAG AA contrast targets (4.5:1 normal, 3:1 large/UI boundaries)
- Respects hierarchy while maintaining readability

**Consequences**:
- Positive: Settings screen readable in all states and both themes
- Positive: Focus indicators visible for keyboard navigation
- Positive: Disabled controls remain distinguishable and understandable
- Negative: More stylesheet maintenance surface

**Related Decisions**: Decision 022 (Explicit QPalette), Decision 034 (Tooltip palette)
**Related Issues**: Stage 3 of 0.4.1 release

---

### Decision 036: Application Icon Identity for Wayland/KDE

**Date**: 2026-11-13
**Status**: Accepted
**Context**: Application icon was set via QApplication but Wayland/KDE Plasma requires
additional identity configuration (StartupWMClass, desktop file name) for correct
taskbar grouping and launcher display.

**Alternatives Considered**:
- Rely on QApplication icon only: incomplete for Wayland
- Set QGuiApplication::setDesktopFileName(): required but not sufficient
- Full identity configuration: desktop file name, StartupWMClass, window icon, resource icon

**Decision**: 
- `Application.cpp`: Added `QGuiApplication::setDesktopFileName("ai-workspace")`
- `MainWindow.cpp`: Explicit `setWindowIcon()` with resource icon
- `ai-workspace.desktop`: Added `StartupWMClass=ai-workspace`
- All 7 hicolor icon sizes (16-256) installed via RPM

**Rationale**:
- Complete Wayland/KDE application identity
- Consistent icon in window titlebar, taskbar, launcher, window switcher
- Desktop file identity matches Qt application identity

**Consequences**:
- Positive: Correct icon and grouping in KDE Plasma Wayland
- Positive: All standard icon sizes available after RPM install
- Negative: Requires all icon sizes to be maintained

**Related Decisions**: Decision 027 (Versioned Session Schema)
**Related Issues**: Stage 4 of 0.4.1 release

---

### Decision 037: Authoritative Application Version Source

**Date**: 2026-11-14
**Status**: Accepted
**Context**: The application version was hardcoded in `Application.cpp` while `CMakeLists.txt` had the project version. This led to inconsistencies where the RPM package version differed from the About screen version. There was no single authoritative source.

**Alternatives Considered**:
- Keep hardcoded version in Application.cpp: simple but error-prone
- Use Git tags to derive version: requires git at build time, complex
- Generate version header from CMake project version: single source of truth

**Decision**: 
- Create `src/core/version.h.in` template with `@PROJECT_VERSION@` placeholder
- Add `configure_file()` in `CMakeLists.txt` to generate `src/core/version.h` from CMake project version
- Update `Application.cpp` to include `version.h` and use `AI_WORKSPACE_VERSION` macro
- Version is now defined in exactly one place: `CMakeLists.txt` project() command

**Rationale**:
- Single authoritative version source eliminates drift
- CMake project version drives RPM spec, runtime version, About dialog, and `--version` flag
- Clean build from fresh directory always produces correct version
- No manual synchronization needed

**Consequences**:
- Positive: Version consistency guaranteed across RPM, runtime, About dialog
- Positive: No stale version strings in runtime-facing metadata
- Positive: Build system enforces version synchronization
- Negative: Requires CMake configure step to generate header (standard practice)

**Related Decisions**: Decision 027 (Versioned Session Schema)
**Related Issues**: Stage 1 of 0.4.2 release

---

### Decision 038: Collapsed Sidebar Represents Open Tabs

**Date**: 2026-11-14
**Status**: Accepted
**Context**: The collapsed sidebar previously displayed permanent provider shortcuts (9 icons for 9 providers). Once tabs were opened, this was not useful because the user couldn't see which providers their open tabs belonged to.

**Alternatives Considered**:
- Keep permanent provider shortcuts: simple but not useful with open tabs
- Show both provider shortcuts and tab icons: cluttered, redundant
- Replace with open tab icons: most useful, shows actual state

**Decision**: Replace permanent provider shortcuts with icons representing currently open tabs. Each open tab gets its own icon in the collapsed rail, in the same order as the Open Tabs list.

**Rationale**:
- User sees exactly what tabs are open at a glance
- No redundant provider shortcuts when tabs exist
- Collapsed rail becomes a functional tab switcher
- Consistent with expanded sidebar where Open Tabs list shows actual tabs

**Consequences**:
- Positive: Collapsed sidebar shows actionable information (open tabs)
- Positive: No duplicate provider identity when tabs are open
- Negative: Empty state shows no provider shortcuts (user must expand to see all providers)
- Negative: Must open a tab before its provider appears in collapsed rail

**Related Decisions**: Decision 029 (Horizontal Provider Strip), Decision 032 (Workspace Header Removal)
**Related Issues**: Stage 2 of 0.4.2 release

---

### Decision 039: Duplicate-Provider Tabs Receive Separate Rail Entries

**Date**: 2026-11-14
**Status**: Accepted
**Context**: When multiple tabs belong to the same provider (e.g., two DeepSeek chats), the collapsed rail could either deduplicate them into one icon or show separate icons for each tab.

**Alternatives Considered**:
- Deduplicate by provider: one icon per provider, click shows menu of tabs
- Separate icon per tab: each tab gets its own icon, even if same provider

**Decision**: Each tab gets its own icon in the collapsed rail. No deduplication by provider.

**Rationale**:
- Direct one-click activation of any tab without intermediate menu
- Visual position corresponds to tab order
- Tooltip identifies both provider and specific tab
- Consistent with expanded Open Tabs list where each tab is a separate row

**Consequences**:
- Positive: Fastest possible tab switching (single click)
- Positive: Visual tab count matches actual tab count
- Positive: No hidden menus or extra clicks
- Negative: Can have many icons from same provider (uses more vertical space)
- Negative: Provider identity alone doesn't distinguish tabs in rail (rely on tooltip)

**Related Decisions**: Decision 038 (Collapsed Sidebar Represents Open Tabs)
**Related Issues**: Stage 2 of 0.4.2 release

---

### Decision 040: Fixed Bottom Control Section in Collapsed Rail

**Date**: 2026-11-14
**Status**: Accepted
**Context**: The collapsed sidebar needs Settings and Expand controls. Previously they were at the bottom of a growing list of provider shortcuts, which could push them off-screen with many tabs.

**Alternatives Considered**:
- Controls at bottom of scrollable rail: could be pushed off-screen
- Controls in fixed overlay: visually disconnected from rail
- Dedicated bottom control section outside scroll area: always accessible

**Decision**: Create a dedicated `collapsedBottomControls` widget with vertical layout (Settings above Expand) that sits outside the scrollable tab rail area. The tab rail is in a scroll area that takes remaining space; bottom controls are in a fixed-position widget at the bottom of the collapsed rail.

**Rationale**:
- Settings and Expand are always accessible regardless of tab count
- Vertical stacking (Settings above Expand) fits the narrow 60px rail
- Controls don't overlap with tab icons
- Clean visual separation between dynamic tab rail and permanent controls

**Consequences**:
- Positive: Controls always visible and clickable
- Positive: No scrolling needed to reach Settings or Expand
- Negative: Slightly less space for tab icons (bottom ~80px reserved for controls)
- Negative: Two-widget layout adds minor complexity

**Related Decisions**: Decision 038 (Collapsed Sidebar Represents Open Tabs)
**Related Issues**: Stage 3 of 0.4.2 release

---

### Decision 041: Developer Profile URL Policy

**Date**: 2026-11-14
**Status**: Accepted
**Context**: The application needs a developer link in the About screen. The project has not been published on GitHub yet, but the developer has a GitHub profile at `https://github.com/ChiefDenis/`.

**Alternatives Considered**:
- Link to non-existent repository: misleading
- No developer link: misses attribution opportunity
- Link to developer profile: accurate, honest about publication status

**Decision**: Use the developer's GitHub profile URL (`https://github.com/ChiefDenis/`) with label "GitHub" in the About dialog. Update RPM spec URL field to match. Do not invent a repository URL or claim the project is published on GitHub.

**Rationale**:
- Honest about publication status (profile ≠ repository)
- Provides a way to contact/find the developer
- Consistent across About dialog and RPM metadata
- Avoids misleading users about project hosting

**Consequences**:
- Positive: Accurate developer attribution
- Positive: No false claims about GitHub repository
- Negative: Users expecting a repository will find a profile instead
- Negative: Must update if/when project is published to a repository

**Related Decisions**: Decision 036 (Application Icon Identity)
**Related Issues**: Stage 4 of 0.4.2 release

---

### Decision 042: Canonical Collapsed Sidebar Bottom Controls

**Date**: 2026-11-14
**Status**: Accepted
**Context**: The collapsed sidebar had duplicate Settings and Expand controls because `setupBottomSection()` created a bottom section in the main sidebar layout (always visible), while `setupCollapsedRail()` created its own controls inside the collapsed rail.

**Alternatives Considered**:
- Hide the main bottom section when collapsed: complex visibility management
- Remove collapsed rail's bottom controls: loses Settings/Expand in collapsed mode
- Move expanded sidebar's bottom controls inside the expanded container: clean separation

**Decision**: Remove `setupBottomSection()` entirely. Create `setupExpandedBottomSection()` that adds Settings and Collapse buttons inside the expanded container. Keep `setupCollapsedRail()` with its own Settings and Expand buttons inside the collapsed rail.

**Rationale**:
- Clean separation: expanded sidebar controls only visible when expanded, collapsed rail controls only visible when collapsed
- No duplicate widgets in widget hierarchy
- Simpler `setCollapsed()` logic - only needs to toggle container visibility
- Maintains Settings above Expand layout in both modes

**Consequences**:
- Positive: No duplicate controls, cleaner widget hierarchy
- Positive: Simpler collapse/expand logic
- Negative: Slightly more code (two separate bottom section setups)

**Related Decisions**: Decision 038 (Collapsed Sidebar Represents Open Tabs), Decision 040 (Fixed Bottom Control Section)
**Related Issues**: Stage 1 of 0.4.3 release

---

### Decision 043: Reproducible RPM Build Pipeline with Explicit Asset Installation

**Date**: 2026-11-14
**Status**: Accepted
**Context**: The RPM build was unreliable - it built tests (slow), and CMake install rules for desktop file and icons were not executing in the RPM build environment.

**Alternatives Considered**:
- Fix CMake install rules: complex, platform-specific issues
- Build tests in RPM: slow, unnecessary for binary package
- Explicit installation in RPM spec: reliable, standard practice

**Decision**: 
- Add `BUILD_TESTS` CMake option (default ON for development, OFF for RPM)
- RPM spec builds with `-DBUILD_TESTS=OFF` and only targets `ai-workspace`
- Explicit `desktop-file-install` and icon installation loops in RPM spec `%install` section
- Include scalable SVG icon

**Rationale**:
- RPM builds faster (no tests)
- Explicit installation is reliable and auditable
- Standard Fedora packaging practice
- Assets installed to correct FHS locations

**Consequences**:
- Positive: Faster, reliable RPM builds
- Positive: Explicit control over installed files
- Negative: Duplication of install logic (CMake + RPM spec)

**Related Decisions**: Decision 036 (Application Icon Identity), Decision 027 (Versioned Session Schema)
**Related Issues**: Stage 2 of 0.4.3 release

---

### Decision 044: KDE Application Icon Installation and Desktop Identity

**Date**: 2026-11-14
**Status**: Superseded by Decision 046 (icon naming); installation and desktop identity parts remain valid
**Context**: The installed application showed a generic icon in KDE Plasma launcher and taskbar. The desktop file and icons were not reliably installed by CMake in the RPM build.

**Alternatives Considered**:
- Rely on CMake install rules: wasn't working in RPM build environment
- Add icon installation to RPM spec: reliable, explicit
- Use GTK icon cache update: standard freedesktop mechanism

**Decision**:
- Explicit `desktop-file-install` in RPM spec
- Explicit icon installation loops for 7 PNG sizes (16-256) + scalable SVG
- `%post`/`%postun` scripts run `gtk-update-icon-cache` and `update-desktop-database`
- Desktop file: `Icon=ai-workspace`, `StartupWMClass=ai-workspace`
- Application: `QGuiApplication::setDesktopFileName("ai-workspace")`, `setWindowIcon()`

**Rationale**:
- Explicit installation ensures files reach correct FHS locations
- Standard icon cache update works for both GTK and Qt applications
- Complete Wayland/KDE application identity

**Consequences**:
- Positive: Correct icon in launcher, taskbar, window
- Positive: Reliable across packaging scenarios
- Negative: More spec file complexity

**Related Decisions**: Decision 036 (Application Icon Identity), Decision 043 (RPM Build Pipeline)
**Related Issues**: Stage 3 of 0.4.3 release

---

### Decision 045: KDE Icon Cache Update via hicolor-icon-theme Transfiletriggers

**Date**: 2026-11-14
**Status**: Superseded — the cache hypothesis was disproven in 0.4.5; see Decision 046
**Context**: The 0.4.3 RPM correctly installed all icon files and desktop entry, but the installed application still showed a generic icon in KDE Plasma. The hicolor icon theme cache (`.icon-theme.cache`) was not being created after RPM installation, so KDE couldn't find the icon by name.

**Alternatives Considered**:
- Add manual `gtk-update-icon-cache` call in RPM `%post`: works but duplicates what hicolor-icon-theme transfiletriggers already do
- Rely on hicolor-icon-theme transfiletriggers: standard mechanism but requires the package to be installed and triggers to fire
- Add `--force` flag to `gtk-update-icon-cache` in RPM `%post`: ensures cache is rebuilt even if timestamps are unusual

**Decision**:
- Keep CMake install rules for desktop file and icons (single source of truth)
- Rely on hicolor-icon-theme transfiletriggers for automatic icon cache updates
- Add `--force` flag to `gtk-update-icon-cache` in RPM `%post`/`%postun` for robustness
- Add explicit `Requires: hicolor-icon-theme` to ensure transfiletriggers are available
- Remove redundant manual icon installation from RPM spec (use CMake install only)

**Rationale**:
- Transfiletriggers are the standard Fedora mechanism for icon cache updates
- `--force` flag handles edge cases where file timestamps don't trigger rebuild
- Explicit `Requires: hicolor-icon-theme` guarantees trigger availability
- Single installation path (CMake) eliminates duplication and conflicts

**Consequences**:
- Positive: Automatic icon cache updates via standard mechanism
- Positive: Robust cache rebuild with `--force` flag
- Positive: Cleaner spec file with single installation path
- Negative: Requires hicolor-icon-theme package to be installed (standard on Fedora)

**Related Decisions**: Decision 036 (Application Icon Identity), Decision 043 (RPM Build Pipeline), Decision 044 (KDE Application Icon Installation)
**Related Issues**: Stage 5 of 0.4.4 release

---

### Decision 046: Hyphen-Free Application Icon Name for KDE

**Date**: 2026-11-14
**Status**: Accepted
**Context**: Releases 0.4.3 and 0.4.4 still showed a generic icon. System inspection of the 0.4.4 installation proved that the files, desktop entry, Qt resource, startup order and icon cache were all correct. The actual defect was in KDE's icon lookup:

- `KIconLoaderPrivate::findMatchingIcon()` (KF6 6.30) loops over icon themes and, within each theme, removes the last `-`-separated suffix until something matches. Only then does it continue to the next inherited theme.
- The active theme was Kora. Kora ships `mimetypes/scalable/ai.svg` for Adobe Illustrator. For the name `ai-workspace`, Kora had no exact match, so KIconLoader stripped `-workspace` and matched `ai` before it ever consulted hicolor's exact `ai-workspace.png`.
- `kiconfinder6 ai-workspace` returned `~/.local/share/icons/kora/mimetypes/scalable/ai.svg`; `kiconfinder6 ai_workspace` returned the packaged hicolor PNG when installed in an isolated data dir.
- A native Qt 6.10.3/KDE Wayland probe confirmed `QIcon::fromTheme("ai-workspace")` returned `name=ai` with center pixel `#6f350c` (the Illustrator icon), while `QIcon::fromTheme("ai_workspace")` returned the orange Sooyah artwork.
- A stale hicolor cache did not prevent `ai_workspace` from resolving, so the 0.4.4 cache hypothesis was wrong.

**Alternatives Considered**:
- Rebuild the icon cache again: disproven; the exact hicolor icon was present and cached, but KIconLoader never reached it.
- Install the icon into every theme: unmaintainable and still theme-dependent.
- Use an absolute icon path in the desktop entry: brittle and loses theme/HiDPI integration.
- Use a hyphen-free icon name: matches KDE's lookup rules and keeps normal hicolor lookup.

**Decision**:
- Canonical application icon name is `ai_workspace` (no `-`).
- Desktop entry uses `Icon=ai_workspace`; the desktop file ID and Wayland app_id remain `ai-workspace` so grouping and existing launchers keep working.
- Install `ai_workspace.png` at 16/24/32/48/64/128/256 and `ai_workspace.svg` in hicolor.
- Install only the canonical name; do not ship the known-broken hyphenated alias.
- Keep `StartupWMClass=ai-workspace` and `QGuiApplication::setDesktopFileName("ai-workspace")`.
- Rely on the `hicolor-icon-theme` and `desktop-file-utils` transfiletriggers instead of duplicate RPM scriptlets.

**Rationale**:
- The icon name is the only thing KIconLoader hyphen-splits; removing the hyphen from that one identifier fixes every KDE surface (launcher, taskbar, window decoration, app switcher) while preserving the app_id.
- Keeping the desktop file ID avoids breaking pinned launchers and taskbar grouping.
- Tests can now enforce the invariant that the icon name contains no `-`.

**Consequences**:
- Positive: launcher, taskbar and window resolve the Sooyah artwork in Kora and other themes.
- Positive: no dependence on cache state, build directory or theme overrides.
- Negative: external configurations that referenced the literal icon name `ai-workspace` must use `ai_workspace`.
- Neutral: the desktop file ID and executable name remain `ai-workspace`.

**Related Decisions**: Decision 036 (Application Icon Identity), Decision 044 (KDE Application Icon Installation), Decision 045 (superseded cache hypothesis)
**Related Issues**: 0.4.5 icon root-cause fix

---

### Decision 047: Multai Rebrand and One-Time Legacy Data Migration

**Date**: 2026-11-15
**Status**: Accepted
**Context**: Release 0.5.0 renames the product from AI Workspace to Multai, including the executable, package, desktop entry, Qt identity, icons, QSettings organization and profile paths. Existing users already have authentication data, open tabs, preferences and WebEngine storage under `ai-workspace` paths. The rename must not discard or silently orphan that data.

**Alternatives Considered**:
- Do not migrate and require users to sign in again: unacceptable; loses sessions and tab state.
- Move the legacy directories into place: simple but loses the rollback path and can leave a half-moved tree after interruption.
- Copy directly into the new root: risks overwriting an existing Multai installation and is not atomic.
- Copy into a verified staging directory and atomically rename: preserves the legacy tree, detects partial copies, and is idempotent.

**Decision**:
- Canonical identity is `Multai` / `multai`; the executable, RPM package, desktop file, Wayland app ID, X11 WM class and icon theme name are all `multai`.
- The icon name stays hyphen-free; the new artwork is generated from one canonical SVG into every PNG size.
- `DataMigration` runs at the start of the `Application` constructor, before `QSettings` or profile access.
- Per root (config, data, cache), copy the legacy tree into `<new-root>.migrating`, rename `AI Workspace.conf` to `Multai.conf` and `AI Workspace/` to `Multai/`, verify the complete file map and sizes, write `.migration-complete`, then atomically rename the staging directory to the new root.
- Never overwrite an existing Multai root; skip that root and leave the legacy data untouched.
- Keep the legacy roots after migration so the user can roll back by removing the new roots.
- A stale `.migrating` directory is discarded and rebuilt on the next launch; once the new root exists, migration never runs again.

**Rationale**:
- File-map verification catches truncated or incomplete copies before the new root becomes visible.
- The sibling staging directory allows an atomic rename on the same filesystem.
- Leaving the legacy data in place is the safest rollback path and avoids destructive migration behaviour.
- Running before settings access guarantees the new identity never reads an empty configuration first.

**Consequences**:
- Positive: existing accounts, sessions, tabs, preferences and WebEngine data survive the rename.
- Positive: migration is idempotent, interruption-safe and never overwrites Multai data.
- Negative: legacy data temporarily occupies disk space until the user removes it manually.
- Neutral: the repository directory remains `ai-workspace`; that path is not part of the installed product identity.

**Related Decisions**: Decision 046 (Hyphen-Free Application Icon Name for KDE), Decision 013 (QSettings INI Format for Portability)
**Related Issues**: 0.5.0 rebrand and identity migration

---

## Decision Review Process

### When to Review Decisions

- When new information contradicts the decision
- When the decision causes significant problems
- When requirements change
- When a better alternative becomes available
- Periodically (every 6 months)

### Review Process

1. Identify the decision to review
2. Gather new information or context
3. Re-evaluate alternatives
4. Decide whether to:
   - Keep the decision (update rationale)
   - Modify the decision (update consequences)
   - Reverse the decision (mark as deprecated, create new decision)
5. Update this document
6. Communicate changes to team

### Decision Lifecycle

- **Proposed**: Decision is being considered
- **Accepted**: Decision has been made and implemented
- **Rejected**: Decision was considered but not chosen
- **Deprecated**: Decision was accepted but is no longer valid
- **Reversed**: Decision was accepted but has been reversed

## Decision Categories

### Architecture Decisions

High-level architectural choices:
- Decision 001: Qt 6.10.3 as Minimum Version
- Decision 002: Qt Widgets over Qt Quick
- Decision 003: CMake Build System
- Decision 006: Single Process Architecture
- Decision 018: LifecycleTarget Interface Decouples Automation from Qt WebEngine
- Decision 021: No Separate TabManager Class

### Technology Decisions

Technology stack choices:
- Decision 004: QSettings for Configuration
- Decision 005: SQLite for Structured Data

### Design Decisions

UI/UX design choices:
- Decision 007: Libadwaita-Inspired Design with Qt Widgets
- Decision 008: Profile Storage in User Config Directory
- Decision 019: Provider Icons Generated at Runtime
- Decision 022: Explicit QPalette Alongside Stylesheets
- Decision 023: Painted Application-Owned Toolbar Icons
- Decision 024: ThemedDialog Paints Its Own Background
- Decision 026: Provider Display Names as Tab Titles
- Decision 029: Horizontal Provider Strip with Painted Buttons
- Decision 030: Remove the Workspace Navigation Toolbar
- Decision 031: Bundle Authentic Provider SVG Logos and Render Them as Theme-Aware Icons
- Decision 032: Remove the Remaining Workspace Provider Header
- Decision 033: Remove Sidebar Branding Header
- Decision 034: Application-Wide Tooltip Palette
- Decision 035: Settings Screen Contrast Audit and Refinement
- Decision 036: Application Icon Identity for Wayland/KDE
- Decision 037: Authoritative Application Version Source
- Decision 038: Collapsed Sidebar Represents Open Tabs
- Decision 039: Duplicate-Provider Tabs Receive Separate Rail Entries
- Decision 040: Fixed Bottom Control Section in Collapsed Rail
- Decision 041: Developer Profile URL Policy
- Decision 042: Canonical Collapsed Sidebar Bottom Controls
- Decision 043: Reproducible RPM Build Pipeline with Explicit Asset Installation
- Decision 044: KDE Application Icon Installation and Desktop Identity
- Decision 045: KDE Icon Cache Update via hicolor-icon-theme Transfiletriggers (superseded)
- Decision 046: Hyphen-Free Application Icon Name for KDE

### Implementation Decisions

Implementation approach choices:
- Decision 009: Tab State Machine Separate from Qt Lifecycle State
- Decision 010: Lazy Page Creation
- Decision 011: Respect Qt's recommendedState by Default
- Decision 012: LRU Selection for Resource Allocation
- Decision 013: QSettings INI Format for Portability
- Decision 014: Memory Pressure Detection via /proc/meminfo with Optional PSI
- Decision 015: Profile Cleanup by Recursive Directory Removal
- Decision 016: ProfileManager is the Single Owner of Profile Objects
- Decision 017: Provider Embedding Compatibility Detection (proposed, still open)
- Decision 020: recommendedState Gate is User-Configurable
- Decision 025: State-Asserted Screenshot Harness with WebEngine Compositing
- Decision 027: Versioned Session Schema in QSettings
- Decision 028: Restore Tabs Unloaded and Activate Only the Saved Selection

## Decision Impact Analysis

### High Impact Decisions

Decisions that significantly affect the project:

- **Decision 001 (Qt 6.10.3)**: Affects platform support and availability
- **Decision 002 (Qt Widgets)**: Affects entire UI implementation
- **Decision 009 (Separate State Machine)**: Affects core lifecycle implementation

### Medium Impact Decisions

Decisions that affect specific areas:

- **Decision 004/005 (QSettings/SQLite)**: Affects data persistence
- **Decision 007 (Libadwaita Design)**: Affects UI appearance
- **Decision 010 (Lazy Page Creation)**: Affects performance

### Low Impact Decisions

Decisions that are easily reversible:

- **Decision 013 (INI Format)**: Easy to change
- **Decision 012 (LRU Selection)**: Could be changed to other strategies

## Related Documentation

- [SPEC.md](../SPEC.md) - Requirements that drove these decisions
- [ARCHITECTURE.md](../ARCHITECTURE.md) - Architecture based on these decisions
- [AGENTS.md](../AGENTS.md) - Working protocol influenced by these decisions
