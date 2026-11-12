# Testing Strategy

## Document Information

- **Version**: 1.0
- **Date**: 2026-10-10
- **Status**: Initial Draft
- **Related Documents**: SPEC.md, ARCHITECTURE.md, AGENTS.md

## Overview

This document describes the testing strategy for the AI Workspace project, including unit tests, integration tests, performance tests, and manual testing procedures.

## Testing Philosophy

### Principles

1. **Test early, test often**: Write tests alongside implementation
2. **Test in isolation**: Unit tests should not depend on external systems
3. **Test the interface, not implementation**: Focus on public APIs and behaviours
4. **Test edge cases**: Don't just test happy paths
5. **Measure actual results**: Never invent successful test results
6. **Automate where possible**: Use CTest for automated test execution
7. **Manual testing for UI**: Qt Widgets require manual UI testing

### Testing Levels

1. **Unit Tests**: Test individual components in isolation
2. **Integration Tests**: Test component interactions
3. **Performance Tests**: Measure resource usage and timings
4. **Manual Tests**: Test UI and user workflows

## Test Framework

### Framework Selection

- **Unit Tests**: Qt Test Framework (Qt Test)
- **Integration Tests**: Qt Test Framework
- **Performance Tests**: Custom measurement tools with Qt Test
- **Test Runner**: CTest (CMake's test runner)

### Test Organization

```
tests/
├── unit/
│   ├── test_tabmanager.cpp
│   ├── test_lifecyclemanager.cpp
│   ├── test_profilemanager.cpp
│   ├── test_providerregistry.cpp
│   ├── test_tabstatemachine.cpp
│   └── test_settingsmanager.cpp
├── integration/
│   ├── test_lifecycle_integration.cpp
│   ├── test_profile_isolation.cpp
│   ├── test_persistence.cpp
│   └── test_browser_integration.cpp
└── performance/
    ├── test_memory_usage.cpp
    ├── test_tab_timings.cpp
    └── test_profile_growth.cpp
```

## Unit Tests

### Test Coverage Goals

- **Overall coverage**: >= 80%
- **Critical components**: >= 90%
- **UI components**: >= 60% (harder to test)

### TabManager Tests

**File**: `tests/unit/test_tabmanager.cpp`

**Test Cases**:

1. **Tab Creation**
   - Create tab with valid provider and profile
   - Create tab with null provider (should fail)
   - Create tab with null profile (should fail)
   - Verify tabCreated signal emitted
   - Verify tab metadata stored correctly

2. **Tab Destruction**
   - Destroy existing tab
   - Destroy non-existent tab (should fail gracefully)
   - Verify tabDestroyed signal emitted
   - Verify metadata cleaned up

3. **Tab Activation**
   - Activate existing tab
   - Activate already active tab (idempotent)
   - Verify tabActivated signal emitted
   - Verify lifecycle manager notified

4. **Tab Deactivation**
   - Deactivate active tab
   - Deactivate inactive tab (should fail gracefully)
   - Verify tabDeactivated signal emitted
   - Verify lifecycle manager notified

5. **Tab Persistence**
   - Save tab configuration
   - Restore tab configuration
   - Verify metadata restored correctly
   - Verify provider and profile references restored

6. **Edge Cases**
   - Create maximum number of tabs
   - Create tab with same provider and profile
   - Handle tab manager destruction with active tabs

**Acceptance Criteria**:
- All tests pass
- No memory leaks (verify with valgrind)
- Signal/slot connections work correctly

### LifecycleManager Tests

**File**: `tests/unit/test_lifecyclemanager.cpp`

**Test Cases**:

1. **State Transitions**
   - Transition from Unloaded to Loading
   - Transition from Loading to Active
   - Transition from Active to Frozen
   - Transition from Frozen to Active
   - Transition from Frozen to Discarded
   - Transition from Discarded to Loading
   - Attempt invalid transitions (should fail)

2. **Freeze Timer**
   - Start freeze timer on tab deactivation
   - Cancel freeze timer on tab activation
   - Verify freeze after delay
   - Verify freeze not triggered if tab reactivated

3. **Hibernate Timer**
   - Start hibernate timer after freeze
   - Cancel hibernate timer on tab activation
   - Verify hibernate after delay
   - Verify hibernate not triggered if tab reactivated

4. **LRU Selection**
   - Track tab activation order
   - Select least recently used tab
   - Handle empty tab list
   - Handle single tab

5. **Memory Pressure**
   - Simulate memory pressure signal
   - Verify hibernation triggered
   - Verify LRU selection for hibernation
   - Verify no hibernation if no eligible tabs

6. **Recommended State**
   - Respect recommendedState from Qt
   - Allow user override (if configured)
   - Document side effects of override

**Acceptance Criteria**:
- All state transitions work correctly
- Timers fire at correct intervals
- LRU selection works correctly
- Memory pressure response works correctly
- recommendedState respected by default

### ProfileManager Tests

**File**: `tests/unit/test_profilemanager.cpp`

**Test Cases**:

1. **Profile Creation**
   - Create profile with valid name
   - Create profile with empty name (should fail)
   - Create profile with duplicate name (should fail)
   - Verify profileCreated signal emitted
   - Verify profile directory created
   - Verify profile permissions correct (0700)

2. **Profile Deletion**
   - Delete existing profile
   - Delete non-existent profile (should fail gracefully)
   - Verify profileDeleted signal emitted
   - Verify profile directory removed
   - Verify QWebEngineProfile cleaned up

3. **Profile Renaming**
   - Rename existing profile
   - Rename to existing name (should fail)
   - Verify profileRenamed signal emitted
   - Verify directory renamed

4. **Profile Retrieval**
   - Get profile by ID
   - Get profiles for provider
   - Handle non-existent profile
   - Handle provider with no profiles

5. **Profile Isolation**
   - Verify separate directories
   - Verify separate QWebEngineProfile instances
   - Verify no cross-profile data access

**Acceptance Criteria**:
- Profile creation/deletion works correctly
- Profile permissions are correct
- Profile isolation works
- No data leakage between profiles

### ProviderRegistry Tests

**File**: `tests/unit/test_providerregistry.cpp`

**Test Cases**:

1. **Provider Loading**
   - Load all 9 providers
   - Verify provider count
   - Verify provider IDs are unique
   - Verify provider URLs are valid

2. **Provider Retrieval**
   - Get provider by ID
   - Get provider by invalid ID (should return null)
   - Get all providers
   - Verify provider metadata

3. **Provider Validation**
   - Validate provider URLs
   - Validate provider icons exist
   - Validate provider display names

**Acceptance Criteria**:
- All 9 providers load correctly
- Provider retrieval works
- Provider validation passes

### TabStateMachine Tests

**File**: `tests/unit/test_tabstatemachine.cpp`

**Test Cases**:

1. **State Definitions**
   - Verify all 6 states defined
   - Verify state enum values

2. **State Validity**
   - Check valid transitions for each state
   - Check invalid transitions are rejected
   - Verify state transition guards

3. **State Persistence**
   - Serialize state to string
   - Deserialize state from string
   - Verify round-trip consistency

**Acceptance Criteria**:
- All states defined correctly
- Valid transitions enforced
- State serialization works

### SettingsManager Tests

**File**: `tests/unit/test_settingsmanager.cpp`

**Test Cases**:

1. **Settings Storage**
   - Save boolean setting
   - Save integer setting
   - Save string setting
   - Verify values persist

2. **Settings Retrieval**
   - Retrieve existing setting
   - Retrieve non-existent setting (use default)
   - Verify default values

3. **Settings Signals**
   - Verify settingChanged signal emitted
   - Verify signal contains correct key

4. **Settings Reset**
   - Reset setting to default
   - Reset all settings
   - Verify defaults restored

**Acceptance Criteria**:
- Settings save and load correctly
- Default values work
- Signals emitted correctly

## Integration Tests

### Lifecycle Integration Tests

**File**: `tests/integration/test_lifecycle_integration.cpp`

**Test Cases**:

1. **Qt Lifecycle API Integration**
   - Set Qt lifecycleState to Frozen
   - Verify application state synchronises
   - Set Qt lifecycleState to Discarded
   - Verify application state synchronises
   - Monitor lifecycleStateChanged signal
   - Monitor recommendedStateChanged signal

2. **Full Lifecycle Flow**
   - Create tab (Unloaded)
   - Activate tab (Loading → Active)
   - Deactivate tab (timer started)
   - Wait for freeze (Active → Frozen)
   - Wait for hibernate (Frozen → Discarded)
   - Reactivate tab (Discarded → Loading → Active)
   - Verify all transitions work

3. **Renderer Crash Integration**
   - Simulate renderProcessTerminated signal
   - Verify automatic reload attempted
   - Verify error recovery works

**Acceptance Criteria**:
- Qt lifecycle API integrates correctly
- Full lifecycle flow works end-to-end
- Renderer crash recovery works

### Profile Isolation Tests

**File**: `tests/integration/test_profile_isolation.cpp`

**Test Cases**:

1. **Cookie Isolation**
   - Set cookie in profile A
   - Verify cookie not in profile B
   - Verify cookie persists in profile A

2. **Local Storage Isolation**
   - Set local storage in profile A
   - Verify not accessible in profile B
   - Verify persists in profile A

3. **Authentication Isolation**
   - Log in to provider in profile A
   - Verify not logged in in profile B
   - Verify remains logged in in profile A

**Acceptance Criteria**:
- Cookies isolated between profiles
- Local storage isolated
- Authentication isolated

### Persistence Tests

**File**: `tests/integration/test_persistence.cpp`

**Test Cases**:

1. **Tab Metadata Persistence**
   - Create multiple tabs
   - Save metadata to database
   - Close application
   - Reopen application
   - Verify tabs restored correctly

2. **Settings Persistence**
   - Change settings
   - Close application
   - Reopen application
   - Verify settings restored

3. **Profile Persistence**
   - Create profiles
   - Close application
   - Reopen application
   - Verify profiles restored

**Acceptance Criteria**:
- Tab metadata persists correctly
- Settings persist correctly
- Profiles persist correctly

### Browser Integration Tests

**File**: `tests/integration/test_browser_integration.cpp`

**Test Cases**:

1. **Provider Loading**
   - Load each provider in a tab
   - Verify page loads without errors
   - Verify no console errors
   - Verify provider UI is functional

2. **Navigation**
   - Navigate within provider
   - Verify back/forward works
   - Verify URL updates

3. **JavaScript Execution**
   - Verify JavaScript executes
   - Verify no JavaScript errors

**Acceptance Criteria**:
- All 9 providers load
- Navigation works
- JavaScript executes

## Performance Tests

### Memory Usage Tests

**File**: `tests/performance/test_memory_usage.cpp`

**Test Cases**:

1. **Baseline Measurement**
   - Launch application with no tabs
   - Measure memory usage
   - Verify < 200 MB target

2. **Per-Tab Memory (Active)**
   - Create 1 active tab
   - Measure memory increase
   - Verify < 300 MB per tab target
   - Create 5 active tabs
   - Measure total memory
   - Verify scaling is linear

3. **Per-Tab Memory (Frozen)**
   - Freeze tab
   - Measure memory decrease
   - Verify < 100 MB per tab target

4. **Per-Tab Memory (Discarded)**
   - Discard tab
   - Measure memory decrease
   - Verify < 10 MB per tab target

5. **Multi-Tab Memory**
   - Create 10 tabs
   - Freeze all but one
   - Hibernate 5 frozen tabs
   - Measure total memory
   - Verify within acceptable limits

**Measurement Tools**:
- `/proc/<pid>/status` for RSS memory
- `/proc/<pid>/smaps` for detailed breakdown
- Valgrind massif for heap profiling

**Acceptance Criteria**:
- Baseline < 200 MB
- Active tab < 300 MB additional
- Frozen tab < 100 MB additional
- Discarded tab < 10 MB additional

### Tab Timing Tests

**File**: `tests/performance/test_tab_timings.cpp`

**Test Cases**:

1. **Startup Time**
   - Measure time from launch to window visible
   - Verify < 3 seconds target

2. **Tab Creation Time**
   - Measure first tab creation time
   - Verify < 2 seconds target
   - Measure subsequent tab creation time
   - Verify < 1 second target

3. **Tab Selection Time (Active)**
   - Measure time to switch between active tabs
   - Verify < 100 ms target

4. **Tab Activation Time (Frozen)**
   - Measure time to activate frozen tab
   - Verify < 500 ms target

5. **Tab Activation Time (Discarded)**
   - Measure time to activate discarded tab
   - Verify < 2 seconds target (includes reload)

**Measurement Tools**:
- QElapsedTimer for precise timing
- Average over 10 runs per measurement

**Acceptance Criteria**:
- Startup < 3 seconds
- First tab < 2 seconds
- Subsequent tabs < 1 second
- Active selection < 100 ms
- Frozen activation < 500 ms
- Discarded activation < 2 seconds

### Profile Growth Tests

**File**: `tests/performance/test_profile_growth.cpp`

**Test Cases**:

1. **Profile Size Baseline**
   - Create new profile
   - Measure initial size
   - Verify minimal initial size

2. **Profile Growth Over Time**
   - Simulate 1 week of usage
   - Load pages, store data
   - Measure profile size
   - Verify < 50 MB per week target

3. **Cache Growth Over Time**
   - Simulate 1 week of usage
   - Load pages, cache resources
   - Measure cache size
   - Verify < 100 MB per week target

4. **Cache Cleanup**
   - Clear cache
   - Verify cache size reduced
   - Verify profile data preserved

**Measurement Tools**:
- `du -sh` for directory size
- `find` for file count
- Custom measurement in ProfileStorage

**Acceptance Criteria**:
- Profile growth < 50 MB per week
- Cache growth < 100 MB per week
- Cache cleanup works without losing profile data

## Manual Testing

### UI Testing

**Test Cases**:

1. **MainWindow**
   - [ ] Window opens correctly
   - [ ] Window geometry persists
   - [ ] Menu items work
   - [ ] Keyboard shortcuts work

2. **TabWidget**
   - [ ] New tab button works
   - [ ] Tab close button works
   - [ ] Tab switching works
   - [ ] Tab reordering works (if implemented)

3. **ProviderSelectionDialog**
   - [ ] Dialog opens correctly
   - [ ] All providers listed
   - [ ] Provider icons display
   - [ ] Profile selector works
   - [ ] Create button works
   - [ ] Cancel button works

4. **SettingsDialog**
   - [ ] Dialog opens correctly
   - [ ] All settings categories work
   - [ ] Settings save correctly
   - [ ] Apply button works
   - [ ] Cancel button works

5. **ProfileManagementDialog**
   - [ ] Dialog opens correctly
   - [ ] Profiles listed correctly
   - [ ] Create profile works
   - [ ] Rename profile works
   - [ ] Delete profile works
   - [ ] Confirmation dialog shows

### Provider Testing

**Test Cases**:

For each of the 9 providers:

1. **Basic Loading**
   - [ ] Provider loads without errors
   - [ ] Provider UI displays correctly
   - [ ] No console errors
   - [ ] No embedding blocks detected

2. **Authentication**
   - [ ] Login works
   - [ ] Login persists across restarts
   - [ ] Logout works
   - [ ] Multiple accounts work (separate profiles)

3. **Basic Functionality**
   - [ ] Can send message
   - [ ] Can receive response
   - [ ] Can create new conversation
   - [ ] Can view history

### Lifecycle Testing

**Test Cases**:

1. **Freeze Behaviour**
   - [ ] Tab freezes after delay
   - [ ] Frozen tab becomes unresponsive
   - [ ] Frozen tab reactivates on selection
   - [ ] Reactivation is fast (< 500 ms)

2. **Hibernate Behaviour**
   - [ ] Tab hibernates after delay
   - [ ] Hibernated tab discards page
   - [ ] Hibernated tab reloads on selection
   - [ ] Reload is fast (< 2 seconds)

3. **Memory Pressure**
   - [ ] Application responds to memory pressure
   - [ ] Tabs hibernate under pressure
   - [ ] LRU selection works
   - [ ] Application remains stable

### Theme Testing

**Test Cases**:

1. **Light Theme**
   - [ ] Light theme applies correctly
   - [ ] All UI elements visible
   - [ ] Text is readable
   - [ ] Icons are visible

2. **Dark Theme**
   - [ ] Dark theme applies correctly
   - [ ] All UI elements visible
   - [ ] Text is readable
   - [ ] Icons are visible

3. **Theme Switching**
   - [ ] Switching between themes works
   - [ ] Theme persists across restarts
   - [ ] No visual glitches during switch

## Test Execution

### Running All Tests

```bash
# Build tests
cmake --build build

# Run all tests with CTest
ctest --test-dir build --output-on-failure

# Run with verbose output
ctest --test-dir build --verbose

# Run specific test suite by label
ctest --test-dir build -R integration
ctest --test-dir build -R performance
```

### Running Individual Tests

```bash
# Run specific test executable
./build/tests/test_providerregistry
./build/tests/test_settingsmanager
./build/tests/test_lifecycle
./build/tests/test_lifecyclemanager
./build/tests/test_memorymonitor
./build/tests/test_tabstore
./build/tests/test_profilemanager
./build/tests/test_recovery
./build/tests/test_ui_polish
./build/tests/test_settingsdialog
./build/tests/test_sidebar
./build/tests/test_browserworkspace
./build/tests/test_lifecycle_integration
./build/tests/test_profile_isolation
./build/tests/test_persistence
./build/tests/test_browser_integration
./build/tests/test_memory_usage
./build/tests/test_tab_timings
./build/tests/test_profile_growth
./build/tests/test_visual_corrections
```

### Current Test Results (2026-11-12)

| Category | Tests | Status |
|----------|-------|--------|
| Unit | 12 | ✅ All passing |
| Integration | 10 | ✅ All passing |
| Performance | 3 | ✅ All passing |
| **Total** | **25** | ✅ **All passing** |

Total test time: ~40 seconds. Verified in a clean `build-qa` configure and build.

### Visual QA Screenshot Harness

The screenshot harness is a manual QA tool, not a CTest test. It drives the real
application on a Wayland or X11 session and captures six states in light and dark
themes.

```bash
# Build the harness and application
cmake -B build -S . -DCMAKE_BUILD_TYPE=Debug
cmake --build build --target state-launcher

# Capture all 12 states and generate the contact sheet
BUILD_DIR="$PWD/build" tests/visual-qa/capture-all.sh
```

Outputs:

- `artifacts/visual-qa/01-initial-{light,dark}.png`
- `artifacts/visual-qa/02-active-workspace-{light,dark}.png`
- `artifacts/visual-qa/03-multiple-tabs-{light,dark}.png`
- `artifacts/visual-qa/04-sidebar-collapsed-{light,dark}.png`
- `artifacts/visual-qa/05-empty-workspace-{light,dark}.png`
- `artifacts/visual-qa/06-account-management-{light,dark}.png`
- `artifacts/visual-qa/contact-sheet.png`
- Per-state `.log` files with WebEngine diagnostics

The harness asserts the intended state before writing each image (tab count, active tab,
lifecycle `Active`, header visibility, collapsed rail, empty workspace, or shown and
exposed account dialog). If an assertion fails, capture fails rather than writing a
plausible but wrong screenshot.

The web surface is captured by compositing `QWebEngineView::grab()` at the view's mapped
position because `QWidget::render()` does not include the composited WebEngine surface.
Offscreen QPA cannot paint WebEngine content, so a real display server is required.

### UI Regression Tests

`tests/test_visual_corrections.cpp` verifies rendered behaviour rather than stylesheet
strings: painted toolbar icon pixels, provider icon composition, dark sidebar luminance,
status-bar contrast, empty-state icon contrast, collapsed-rail navigation and window
size preservation, tab selection pixels, load failure/timeout state clearing, provider
tab titles, and dark/light dialog surfaces.

```bash
ctest --test-dir build -R test_visual_corrections --output-on-failure
```

Test labels: `integration` and `performance` CTest labels applied for selective execution.

### Performance Measurement Script

For real-world measurements in a graphical session:

```bash
./tests/measure_performance.sh [output-file.json]
```


### Continuous Integration

**Future**: Set up CI pipeline with:

1. Build on every commit
2. Run unit tests
3. Run integration tests
4. Run performance tests
4. Generate coverage report
5. Fail build if tests fail

## Test Reporting

### Test Results Format

CTest outputs test results in XML format. Parse with:

```bash
# Generate XML report
ctest --test-dir build -T Test --output-on-failure

# Generate coverage report (if gcov/lcov available)
cmake --build build --target coverage
```

### Coverage Reporting

**Future**: Use lcov for coverage reports:

```bash
# Generate coverage data
lcov --capture --directory build --output-file coverage.info

# Generate HTML report
genhtml coverage.info --output-directory coverage_html
```

### Performance Reporting

Performance tests output CSV data:

```csv
metric,target,actual,status
baseline_memory_mb,200,185,pass
active_tab_memory_mb,300,275,pass
frozen_tab_memory_mb,100,85,pass
startup_time_s,3,2.5,pass
```

## Test Maintenance

### Updating Tests

When implementation changes:

1. Update unit tests to match new behaviour
2. Add new tests for new features
3. Remove obsolete tests
4. Verify all tests pass
5. Update this document if testing strategy changes

### Test Review

Review tests periodically:

1. Check for flaky tests (intermittent failures)
2. Check for slow tests (improve or mark as slow)
3. Check for redundant tests (remove duplicates)
4. Check for missing test coverage (add tests)

## Known Test Limitations

### Qt WebEngine Testing

- Qt WebEngine requires a display server (X11 or Wayland)
- Tests may fail in headless CI environments
- Solution: Use virtual display (Xvfb) for CI

### Timing Tests

- Timing tests may be flaky on loaded systems
- Solution: Use averages over multiple runs, set reasonable tolerances

### Memory Tests

- Memory measurements vary by system
- Solution: Use relative measurements, set reasonable tolerances

## Acceptance Criteria

The implementation is considered complete when:

1. **Unit Tests**: All unit tests pass with >= 80% coverage
2. **Integration Tests**: All integration tests pass
3. **Performance Tests**: All performance targets met
4. **Manual Tests**: All manual test cases pass
5. **No regressions**: No test failures from previous stages
6. **No memory leaks**: Valgrind reports no leaks
7. **Stable performance**: Performance measurements consistent across runs

## Test Checklist

Before each stage completion:

- [ ] Unit tests written
- [ ] Unit tests pass
- [ ] Integration tests written (if applicable)
- [ ] Integration tests pass (if applicable)
- [ ] Performance tests run (if applicable)
- [ ] Performance targets met (if applicable)
- [ ] Manual tests completed (if applicable)
- [ ] No memory leaks detected
- [ ] Documentation updated (if testing strategy changed)
- [ ] AI_PROGRESS.md updated with actual results
