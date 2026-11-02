# Technical Decisions

## Document Information

- **Purpose**: Track technical decisions and their rationale throughout the project
- **Status**: Active
- **Last Updated**: 2026-10-30
- **Related Documents**: SPEC.md, ARCHITECTURE.md, AGENTS.md

## Overview

This document records significant technical decisions made during the development of AI Workspace. Each decision includes the context, alternatives considered, the decision made, and the rationale.

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
