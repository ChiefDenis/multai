# AI Workspace - Technical Specification

## Document Information

- **Version**: 1.0
- **Date**: 2026-10-10
- **Status**: Initial Draft
- **Target Platform**: Fedora Linux 43 with KDE Plasma

## 1. Project Overview

### 1.1 Purpose

Build a resource-conscious, multi-provider AI desktop application for Fedora Linux with KDE Plasma. The application embeds provider websites in a native desktop window and provides comprehensive browser lifecycle management to minimise resource usage while maintaining responsiveness.

### 1.2 Key Characteristics

- **Resource-conscious**: Designed to use minimal system resources through intelligent browser lifecycle management
- **Multi-provider**: Supports nine major AI providers from initial release
- **Native desktop**: Built with Qt 6 Widgets for native KDE Plasma integration
- **Profile isolation**: Supports multiple accounts per provider through separate persistent browser profiles
- **No API client**: Embeds provider websites directly; does not implement AI API clients unless separately authorised

## 2. Technology Stack

### 2.1 Core Technologies

- **Language**: C++20
- **GUI Framework**: Qt 6 Widgets
- **Web Engine**: Qt WebEngine and Qt WebEngine Core (minimum Qt 6.10.3)
- **Build System**: CMake 3.31+
- **Testing Framework**: CTest and Qt Test
- **Configuration**: QSettings for simple preferences
- **Persistence**: SQLite (only if required for structured workspace persistence)
- **Version Control**: Git

### 2.2 Verified Dependencies

- **Qt 6.10.3**: Confirmed available on Fedora 43
  - qt6-qtbase-devel-6.10.3
  - qt6-qtdeclarative-devel-6.10.3
  - qt6-qtwayland-devel-6.10.3
  - qt6-qttools-devel-6.10.3
  - qt6-qtwebengine-6.10.3 (runtime)
  - qt6-qtwebengine-devel-6.10.3 (available, must be installed)

- **CMake 3.31.11**: Confirmed available
- **GCC 15.3.1**: Confirmed available with C++20 support
- **SQLite 3.50.2**: Confirmed available

### 2.3 Qt WebEngine Lifecycle API

Qt 6.10.3 provides the following lifecycle features (verified from official documentation):

- **QWebEnginePage::LifecycleState** enum:
  - `Active`: Normal state
  - `Frozen`: Low CPU usage state where most HTML task sources are suspended
  - `Discarded`: Very low resource usage state where the entire browsing context is discarded

- **Properties**:
  - `lifecycleState`: Current lifecycle state of the page
  - `recommendedState`: Recommended limit for the lifecycle state (read-only)

- **Signals**:
  - `lifecycleStateChanged(QWebEnginePage::LifecycleState)`
  - `recommendedStateChanged(QWebEnginePage::LifecycleState)`

- **Hard limits enforced by Qt**:
  - A visible page must remain in the Active state
  - If a page is being inspected by devTools, both pages must remain in Active state
  - A page in the Discarded state can only transition to Active state (causes reload)

- **Soft limits (recommendedState)**:
  - Setting lifecycle state to a lower resource usage state than recommended may cause side effects:
    - Stopping background audio playback
    - Loss of HTML form input
  - Setting to a higher resource state is completely safe

## 3. Supported Providers

### 3.1 Provider List

The application must support these nine providers from initial release:

| # | Provider | Display Name | Official URL | Notes |
|---|----------|--------------|--------------|-------|
| 1 | DeepSeek | DeepSeek | https://www.deepseek.com/ | Chinese AI provider |
| 2 | Qwen | Qwen | https://qwen.ai/home | Alibaba Cloud AI |
| 3 | GLM | GLM | https://z.ai | Zhipu AI platform |
| 4 | Microsoft Copilot | Microsoft Copilot | https://copilot.microsoft.com/ | Microsoft's AI assistant |
| 5 | Google Gemini | Google Gemini | https://gemini.google.com/ | Google's AI assistant |
| 6 | Anthropic Claude | Claude | https://claude.ai/ | Anthropic's AI assistant |
| 7 | Perplexity | Perplexity | https://www.perplexity.ai/ | AI-powered search engine |
| 8 | ChatGPT | ChatGPT | https://chatgpt.com/ | OpenAI's chat interface |
| 9 | Kimi | Kimi | https://www.kimi.ai/ | Moonshot AI's assistant |

### 3.2 Provider Configuration Requirements

Each provider must have:

1. **Configurable definition**: Structured configuration in settings
2. **Display name**: Human-readable name for UI
3. **Icon**: Representative icon for provider branding
4. **Official URL**: Verified official website URL (not invented)
5. **Profile assignment**: Ability to assign specific browser profiles

### 3.3 Multi-Account Support

The application must support multiple accounts for a single provider through:

- **Separate persistent browser profiles**: Each account uses its own isolated browser profile
- **Profile management**: UI to create, rename, and delete profiles
- **Profile isolation**: Authentication state, cookies, and site data isolated per profile
- **Profile naming**: User-defined names for profiles (e.g., "Work", "Personal")

### 3.4 Provider Compatibility

The application must:

- **Verify official URLs**: Use only verified official URLs from web search
- **Detect embedding restrictions**: Detect if a provider blocks embedding and inform the user
- **Document limitations**: Clearly document any provider compatibility limitations
- **Respect bot detection**: Never circumvent provider authentication or bot detection
- **Maintain security**: Do not disable browser security to make a provider work

## 4. Browser Lifecycle Architecture

### 4.1 Tab States

Every tab must have an explicit state with the following states and valid transitions:

```
Unloaded → Loading → Active
                 ↓                 ↓
              [error]          Frozen
                 ↓                 ↓
                 ↓              Discarded
                 ↓                 ↓
                 ↓                 ↓
              Closing ←←←←←←←←←←←←←←←
```

#### State Definitions

1. **Unloaded**
   - No live page exists
   - Only metadata and persistent profile remain
   - Tab appears in UI but browser context not created
   - Minimal resource usage (metadata only)

2. **Loading**
   - Page creation or navigation is in progress
   - Browser context exists but page not fully loaded
   - Show loading indicator in UI
   - Cannot be frozen or discarded during this state

3. **Active**
   - Interactive page is fully loaded
   - User can interact with the page
   - All JavaScript and background tasks running
   - Can transition to Frozen when inactive
   - Can transition to Closing when user closes tab

4. **Frozen**
   - Page retained but background execution suspended
   - Most HTML task sources suspended (per Qt documentation)
   - Low CPU usage
   - Page remains visible but not interactive
   - Can transition back to Active when selected
   - Can transition to Discarded after longer inactivity
   - Must respect Qt's recommendedState before freezing

5. **Discarded/Hibernated**
   - Browser context discarded to release resources
   - Entire browsing context removed
   - Only metadata and profile remain
   - Restoration requires page reload
   - Can transition back to Loading (then Active) when selected
   - Should preserve: profile identity, intended URL, tab position, workspace, navigation metadata

6. **Closing**
   - Cleanup in progress
   - Browser context being destroyed
   - All resources being released
   - Terminal state; transitions to Unloaded (removed from UI)

### 4.2 Mandatory Lifecycle Behaviors

#### 4.2.1 Lazy Page Creation

- Create browser pages only when needed (when tab is first selected)
- Do not create pages for all tabs at startup
- Maintain tab metadata in Unloaded state until selection

#### 4.2.2 Automatic Freezing

- Automatically freeze eligible inactive tabs after a **configurable delay**
- Default delay: 5 minutes of inactivity
- Configurable by user in settings
- Only freeze tabs that are:
  - Not visible
  - Not actively loading
  - Not in an active stream (e.g., voice chat, active generation)
  - RecommendedState allows freezing
- Use Qt's recommendedState as guidance; respect it by default

#### 4.2.3 Automatic Hibernation

- Automatically hibernate (discard) eligible inactive tabs after a **longer configurable delay**
- Default delay: 30 minutes of inactivity (after freezing)
- Configurable by user in settings
- Only hibernate tabs that are:
  - Not visible
  - Frozen for the minimum duration
  - Not in a state where discarding would cause data loss
  - RecommendedState allows discarding
- Must preserve all metadata required for restoration

#### 4.2.4 Memory-Pressure-Triggered Hibernation

- Detect system memory pressure on Linux
- Hibernate least-recently-used eligible tabs when memory pressure detected
- Prioritise:
  - Tabs furthest from current selection
  - Tabs that have been frozen longest
  - Tabs not in critical user workflows
- Continue hibernating until memory pressure relieved or no eligible tabs remain

#### 4.2.5 Least-Recently-Used Selection

- Maintain LRU ordering of tabs
- When selecting tabs for freezing or hibernation:
  - Choose the least recently used eligible tab
  - Exclude currently visible tab
  - Exclude tabs actively loading or in critical states
  - Respect recommendedState limits

#### 4.2.6 Immediate Activation

- When a tab is selected:
  - Immediately transition to Loading if Unloaded or Discarded
  - Immediately transition to Active if Frozen
  - Create page context if Unloaded
  - Reload page if Discarded
  - Ensure UI is responsive within 500ms of selection

#### 4.2.7 Complete Cleanup on Close

- When a tab closes:
  - Complete cleanup of application-owned page and view objects
  - Delete QWebEnginePage and QWebEngineView instances
  - Clear references to browser context
  - Transition to Closing, then remove from UI
  - Profile data must persist (authentication, cookies, site data)

#### 4.2.8 Persistence Across Restarts

- Persist tab metadata across application restarts:
  - Tab position and order
  - Assigned provider
  - Assigned profile
  - Intended URL
  - Last known state (Active, Frozen, Discarded, etc.)
  - Navigation history (if feasible)
- Persist browser authentication and site data:
  - Cookies
  - Local storage
  - IndexedDB
  - Site preferences
- Do NOT delete persistent authentication data during cache cleanup

#### 4.2.9 Graceful Recovery

- Handle renderer crashes:
  - Detect renderProcessTerminated signal
  - Attempt to reload the page
  - Show user-friendly error message if reload fails
  - Allow user to retry or close the tab
- Handle failed reloads:
  - Show error in UI
  - Allow manual retry
  - Provide option to reset tab to initial state

### 4.3 Qt WebEngine Lifecycle API Usage

#### 4.3.1 Required API Usage

- Use `QWebEnginePage::lifecycleState` property to control lifecycle
- Use `QWebEnginePage::recommendedState` property as guidance
- Monitor `lifecycleStateChanged` signal for state changes
- Monitor `recommendedStateChanged` signal for recommendation changes
- Monitor `renderProcessTerminated` signal for crash detection

#### 4.3.2 Safety Constraints

- **Never suspend a visible page**: Qt enforces this; violation will fail
- **Never aggressively discard actively loading pages**: Wait for load completion
- **Never discard pages with active streams**: Detect audio/video/activity indicators
- **Respect recommendedState by default**: Only override with clear user documentation of side effects
- **User-configurable overrides**: Must clearly document side effects before allowing override

#### 4.3.3 Implementation Requirements

- Define a clear tab state machine separate from Qt's lifecycle state
- Maintain UI state distinct from actual Qt page lifecycle state
- Synchronize state changes with Qt's lifecycle API
- Handle Qt's enforcement of hard limits gracefully
- Log state transitions for debugging (not for production user data)

### 4.4 Resource Measurement Requirements

#### 4.4.1 Baseline Establishment

Before declaring the application "lightweight", establish:

- **Memory baseline**: Memory usage with no tabs open
- **Per-tab memory**: Memory usage per active tab
- **Per-tab frozen memory**: Memory usage per frozen tab
- **Per-tab discarded memory**: Memory usage per discarded tab
- **Profile size**: Size of persistent profile data per provider
- **Cache size**: Size of disposable cache data
- **Startup time**: Time to launch application with zero tabs
- **Tab creation time**: Time to create and load a new tab
- **Tab activation time**: Time to activate a frozen or discarded tab

#### 4.4.2 Acceptance Thresholds

Define measurable acceptance thresholds:

- **Maximum memory with 5 active tabs**: Must be measured and documented
- **Maximum memory with 10 frozen tabs**: Must be measured and documented
- **Maximum profile growth per week**: Must be measured and documented
- **Maximum cache growth per week**: Must be measured and documented
- **Maximum startup time**: Must be measured and documented
- **Maximum tab activation time**: Must be measured and documented

#### 4.4.3 Monitoring

Implement runtime monitoring:

- Track memory usage per tab
- Track profile size per provider
- Track cache size
- Track renderer process count
- Track active/frozen/discarded tab counts
- Provide optional performance dashboard for debugging

## 5. User Interface Design

### 5.1 Visual Language

Use a Libadwaita-inspired visual language:

- **Clean typography**: Sans-serif fonts, good readability
- **Rounded surfaces**: Rounded corners on windows, dialogs, and containers
- **Carefully spaced controls**: Consistent padding and margins
- **Subtle borders**: Thin, subtle borders between sections
- **Attractive icons**: High-quality icons for providers and actions
- **Restrained colours**: Neutral colour palette with accent colours
- **Polished light/dark themes**: Complete theme support with smooth transitions
- **Adaptive layouts**: Responsive design that adapts to window size

### 5.2 Implementation Technology

- **Framework**: Qt 6 Widgets (NOT Qt Quick/QML)
- **Constraints**:
  - Do NOT introduce GTK/libadwaita libraries
  - Do NOT use Electron
  - Do NOT use JavaScript UI frameworks
  - Do NOT use another GUI framework

### 5.3 Required UI Components

#### 5.3.1 Main Window

- Tab bar for managing multiple provider tabs
- Tab switching interface
- New tab button (provider selector)
- Settings/access to preferences
- Status indicators (lifecycle state, loading, errors)

#### 5.3.2 Tab Interface

- Provider icon and name
- Profile selector (for multi-account support)
- Lifecycle state indicator
- Loading progress indicator
- Error display (when applicable)
- Close button

#### 5.3.3 Provider Configuration Dialog

- List of available providers
- Provider-specific settings (if any)
- Profile management (create, rename, delete)
- Default profile selection

#### 5.3.4 Settings Dialog

- Lifecycle settings:
  - Freeze delay (default: 5 minutes)
  - Hibernate delay (default: 30 minutes)
  - Memory pressure threshold
  - User override options
- Profile settings:
  - Profile storage location
  - Profile cleanup options
- Appearance settings:
  - Theme selection (light/dark/system)
  - Font size
- Advanced settings:
  - Cache limits
  - Debug logging (off by default)

#### 5.3.5 Profile Management UI

- List of profiles per provider
- Create new profile dialog
- Rename profile dialog
- Delete profile confirmation
- Profile usage statistics (optional)

## 6. Resource Constraints

### 6.1 Prohibited Technologies

- **No Electron or Node.js runtime**: Must use native Qt
- **No duplicate browser views**: One tab = one browser view
- **No automatic page reload loops**: Prevent infinite reload cycles
- **No unnecessary polling**: Use event-driven architecture
- **No unbounded resources**:
  - Application logs
  - Histories
  - Queues
  - Retries
  - Timers

### 6.2 Process Constraints

- **No hidden duplicate browser instances**: Ensure one browser process per profile
- **No deletion of persistent authentication data**: Preserve cookies, tokens, session data
- **No indiscriminate site data deletion**: Preserve IndexedDB, local storage, cookies during cache cleanup

### 6.3 Cache and Profile Management

- **Cache limits**: Must be verified against actual Qt APIs and behaviour
- **Profile growth**: Must be measured separately from disposable cache growth
- **Cache cleanup**: Clean only disposable cache, not persistent authentication data
- **Profile cleanup**: User-initiated only, with clear warnings

### 6.4 Network and Telemetry

- **No unrequested telemetry**: No background network activity without user action
- **No background network activity**: No automatic updates, analytics, or phone home
- **Deliberate download handling**: Validate external URLs and handle downloads deliberately

## 7. Security Requirements

### 7.1 Authentication and Access

- **Never circumvent authentication**: Respect provider login mechanisms
- **Never bypass bot detection**: Do not attempt to evade provider security measures
- **Never disable browser security**: Maintain all security features

### 7.2 Data Protection

- **Do not log credentials**: Never log passwords, API keys, tokens
- **Do not log cookies**: Never log cookie contents
- **Do not log sensitive website contents**: Never log chat messages, personal data

### 7.3 Filesystem Permissions

- **Protect profile data**: Use appropriate user-only filesystem permissions (0700 for directories, 0600 for files)
- **Isolate profiles**: Ensure profiles cannot access each other's data

### 7.4 External Content

- **Validate external URLs**: Verify URLs before navigation
- **Handle downloads deliberately**: Require user confirmation for downloads
- **Document limitations**: Clearly document provider compatibility limitations

## 8. Configuration and Persistence

### 8.1 Configuration Storage

- **Simple preferences**: Use QSettings for:
  - User preferences (theme, delays, thresholds)
  - Window geometry and state
  - Provider enable/disable
  - Default profiles

- **Structured data**: Use SQLite only if required for:
  - Complex workspace persistence
  - Tab metadata across restarts
  - Profile management
  - Navigation history

### 8.2 Profile Storage

- **Location**: Separate directory outside application binary
- **Isolation**: Each profile in its own subdirectory
- **Permissions**: User-only (0700 for directories)
- **Structure**: Follow Qt WebEngine profile conventions

### 8.3 Workspace Persistence

Persist across application restarts:

- Tab configuration (provider, profile, URL)
- Tab order and position
- Window geometry and state
- User preferences
- Provider-specific settings

## 9. Testing Requirements

### 9.1 Unit Tests

- Test tab state machine transitions
- Test lifecycle management logic
- Test profile management
- Test configuration persistence
- Test provider configuration

### 9.2 Integration Tests

- Test Qt WebEngine lifecycle API integration
- Test profile isolation
- Test memory pressure response
- Test renderer crash recovery
- Test tab persistence and restoration

### 9.3 Performance Tests

- Measure memory usage per tab state
- Measure tab creation and activation times
- Measure profile and cache growth over time
- Test with multiple tabs (5, 10, 20)
- Test memory pressure scenarios

### 9.4 Manual Testing

- Test each provider for embedding compatibility
- Test multi-account support per provider
- Test theme switching
- Test settings persistence
- Test error recovery

## 10. Performance Requirements

### 10.1 Startup Performance

- Application launch time: < 3 seconds with zero tabs
- First tab creation: < 2 seconds
- Subsequent tab creation: < 1 second

### 10.2 Runtime Performance

- Tab selection (active tab): < 100ms
- Tab activation (frozen): < 500ms
- Tab activation (discarded): < 2 seconds (includes reload)
- UI responsiveness: < 16ms (60 FPS) for UI operations

### 10.3 Memory Performance

- Baseline (no tabs): < 200 MB
- Per active tab: < 300 MB additional
- Per frozen tab: < 100 MB additional
- Per discarded tab: < 10 MB additional (metadata only)

### 10.4 Disk Performance

- Profile growth: < 50 MB per week per active provider
- Cache growth: < 100 MB per week per active provider
- Profile cleanup: User-initiated, clears cache but preserves authentication

## 11. Technical Uncertainties

### 11.1 Profile Ownership

**Uncertainty**: How Qt WebEngine handles profile ownership and cleanup when profiles are created and destroyed.

**Investigation Needed**:
- Does Qt clean up profile data when QWebEngineProfile is destroyed?
- Does profile data persist across application restarts without explicit management?
- What is the correct way to delete a profile and all its data?

### 11.2 Page Lifecycle

**Uncertainty**: Exact behaviour of Qt 6.10.3 lifecycleState and recommendedState API in practice.

**Investigation Needed**:
- What triggers recommendedState changes?
- How does Qt detect audio playback, form input, and other activity?
- What are the actual side effects of freezing/discarding below recommendedState?
- How long does it take to transition between states?

### 11.3 Renderer Cleanup

**Uncertainty**: Process cleanup when pages are discarded.

**Investigation Needed**:
- Does discarding a page immediately terminate the renderer process?
- How many renderer processes are created per profile?
- How to measure actual memory release after discarding?
- Are there orphaned processes or memory leaks?

### 11.4 Disk Growth

**Uncertainty**: Actual profile growth vs cache growth over time.

**Investigation Needed**:
- What data is stored in profiles vs cache?
- How large do profiles grow with typical usage?
- How effective is Qt's automatic cache management?
- What is the optimal cache limit configuration?

### 11.5 Memory Pressure Detection

**Uncertainty**: How to detect and respond to system memory pressure on Linux.

**Investigation Needed**:
- What Linux APIs exist for memory pressure detection?
- How to integrate with Linux memory management?
- What are appropriate thresholds for triggering hibernation?
- How to avoid interfering with system memory management?

### 11.6 Provider Compatibility

**Uncertainty**: Which providers actively block embedding and how to detect this.

**Investigation Needed**:
- Do any providers use X-Frame-Options or CSP to block embedding?
- Do any providers detect WebView/User-Agent and block?
- How to detect when a provider is blocking?
- What workarounds exist (if any are ethical)?

## 12. Implementation Stages

### Stage 1: Project Foundation
- CMake build system
- Basic Qt application skeleton
- QSettings configuration
- Basic UI structure

### Stage 2: Provider System
- Provider configuration structure
- Provider definitions for all 9 providers
- Provider icons
- Provider selection UI

### Stage 3: Profile Management
- Profile creation and management
- Profile isolation
- Profile persistence
- Profile UI

### Stage 4: Basic Browser Integration
- QWebEngineView integration
- Tab management
- Basic navigation
- Provider loading

### Stage 5: Lifecycle State Machine
- Tab state definitions
- State transition logic
- State synchronization with Qt
- State persistence

### Stage 6: Lifecycle Automation
- Automatic freezing logic
- Automatic hibernation logic
- LRU tracking
- Memory pressure detection
- Lifecycle settings UI

### Stage 7: Recovery and Error Handling
- Renderer crash detection
- Failed reload handling
- Graceful error recovery
- Error UI

### Stage 8: UI Polish
- Libadwaita-inspired design
- Theme support (light/dark)
- Responsive layouts
- Icon integration

### Stage 9: Testing and Performance
- Unit tests
- Integration tests
- Performance measurement
- Baseline establishment
- Threshold verification

### Stage 10: Documentation and Packaging
- Complete README
- Installation instructions
- User documentation
- Packaging for Fedora

## 13. Success Criteria

The project is considered successful when:

1. All 9 providers are supported and can be loaded
2. Multi-account support works for at least 3 providers
3. Tab lifecycle states work as specified
4. Automatic freezing and hibernation work correctly
5. Memory pressure response works correctly
6. Renderer crashes are handled gracefully
7. Performance meets or exceeds thresholds
8. All required documentation is complete
9. Tests pass consistently
10. Application can be packaged and installed on Fedora 43

## 14. Non-Requirements

The following are explicitly out of scope:

- AI API client implementation (unless separately authorised)
- Native KDE Plasma widgets (Qt Widgets only)
- Wayland-specific features (use Qt's Wayland support)
- Mobile platforms (desktop only)
- Windows/macOS support (Fedora Linux only)
- Provider-specific integrations (web embedding only)
- Custom AI models or processing
- Voice input/output (unless provider supports it in web interface)
- File upload/download automation (beyond basic browser capabilities)
- Browser extension support
- DevTools integration (unless for debugging)
