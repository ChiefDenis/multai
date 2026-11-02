# Build Mistakes and Fixes

This document records mistakes that broke the build and how they were fixed, to prevent recurrence.

## Mistake 1: Include Path Issues (Stage 1)

**Date**: 2026-10-10
**Stage**: Stage 1 - Project Foundation
**Severity**: Build failure

### Error

```
/home/denis/devprojects/ai-workspace/src/core/Application.cpp:2:10: fatal error: MainWindow.h: No such file or directory
    2 | #include "MainWindow.h"
      |          ^~~~~~~~~~~~~~
```

### Root Cause

- Source files were using includes like `#include "MainWindow.h"` without the correct directory prefix
- CMakeLists.txt didn't have proper include directories configured
- Files in different directories (src/core, src/ui) couldn't find each other's headers

### Fix Applied

1. **Updated CMakeLists.txt** to add include directories:
```cmake
target_include_directories(ai-workspace PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ui
    ${CMAKE_CURRENT_SOURCE_DIR}/src/providers
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)
```

2. **Updated source file includes** to use correct paths:
   - Changed `#include "MainWindow.h"` to `#include "MainWindow.h"` (with include dirs set)
   - Initially tried adding `ui/` prefix but that was wrong approach
   - Correct approach: set include directories in CMake, use simple includes in code

3. **Updated test CMakeLists.txt** similarly:
```cmake
target_include_directories(test_providerregistry PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/core
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/ui
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/providers
    ${CMAKE_CURRENT_SOURCE_DIR}/../src
)
```

### Lessons Learned

- Always configure include directories in CMakeLists.txt for each target
- Use simple includes in source files (e.g., `#include "MainWindow.h"`) when include dirs are set
- Don't hardcode directory prefixes in includes unless necessary
- Test includes must also have proper include directories configured
- Different targets (app vs tests) need separate include directory configurations

### Prevention

- Standardize include directory configuration in CMake
- Always add include directories when creating new targets
- Verify include paths are correct for each source file
- Check that tests can find headers from src directories

---

## Mistake 2: Wrong Standard Icon (Stage 1)

**Date**: 2026-10-10
**Stage**: Stage 1 - Project Foundation
**Severity**: Build failure

### Error

```
/home/denis/devprojects/ai-workspace/src/ui/MainWindow.cpp:66:78: error: 'SP_TabBarAddToNew' is not a member of 'QStyle'
   66 |   QAction* newTabAction = m_toolBar->addAction(style()->standardIcon(QStyle::SP_TabBarAddToNew), "New Tab");
      |                                                                              ^~~~~~~~~~~~~~~~~
```

### Root Cause

- Used `QStyle::SP_TabBarAddToNew` which doesn't exist in Qt 6
- Assumed standard icon would be available without checking Qt documentation

### Fix Applied

Changed to use `QStyle::SP_FileIcon` which is a standard Qt icon:
```cpp
QAction* newTabAction = m_toolBar->addAction(style()->standardIcon(QStyle::SP_FileIcon), "New Tab");
```

### Lessons Learned

- Always verify Qt API exists in the target Qt version before using
- Qt 6 may have different standard icons than Qt 5
- Check Qt documentation for standard icons: https://doc.qt.io/qt-6/qstyle.html#StandardPixmap-enum
- Use common icons like SP_FileIcon, SP_DialogOpenButton when specific icons unavailable

### Prevention

- Reference Qt documentation for specific Qt version being used
- Use generic standard icons when specific ones unavailable
- Consider adding custom icons to resources for specific needs
- Test with the actual Qt version being used (Qt 6.10.3 in this project)

---

## Mistake 3: Wrong Base Class for TabWidget (Stage 1)

**Date**: 2026-10-10
**Stage**: Stage 1 - Project Foundation
**Severity**: Build failure

### Error

```
/home/denis/devprojects/ai-workspace/src/ui/TabWidget.cpp:20:3: error: 'setWidget' was not declared in this scope; did you mean 'TabWidget'?
   20 |   setWidget(0, m_stackedWidget);
      |   ^~~~~~~~~
      |   TabWidget
```

### Root Cause

- Inherited TabWidget from QTabWidget
- Tried to use `setWidget()` which doesn't exist on QTabWidget
- Confused QTabWidget API with other Qt widgets

### Fix Applied

1. Changed TabWidget to inherit from QWidget instead of QTabWidget:
```cpp
class TabWidget : public QWidget {  // Changed from QTabWidget
```

2. Updated constructor to use QVBoxLayout instead of QTabWidget's internal layout:
```cpp
QVBoxLayout* layout = new QVBoxLayout(this);
layout->setContentsMargins(0, 0, 0, 0);
m_stackedWidget = new QStackedWidget(this);
layout->addWidget(m_stackedWidget);
```

3. Updated stylesheets to use QStackedWidget instead of QTabWidget:
```css
QStackedWidget { background-color: #1e1e1e; }
```

### Lessons Learned

- QTabWidget is a complete tab widget with its own bar and pages
- For custom tab behavior, inherit from QWidget and build custom layout
- Don't assume base class methods exist without checking documentation
- TabWidget in this stage is a placeholder for future real tabs (Stage 4)

### Prevention

- Check Qt documentation for base class methods before using
- Choose appropriate base class for intended functionality
- Use simple QWidget for custom layouts, specific widgets for standard behavior
- Document why custom implementation is needed vs using standard widgets

---

## Mistake 4: Multiple Main Functions in Tests (Stage 1)

**Date**: 2026-10-10
**Stage**: Stage 1 - Project Foundation
**Severity**: Linker error

### Error

```
/usr/bin/ld: CMakeFiles/tests.dir/test_settingsmanager.cpp.o: in function `main':
/home/denis/devprojects/ai-workspace/tests/test_settingsmanager.cpp:86: multiple definition of `main'; 
CMakeFiles/tests.dir/test_providerregistry.cpp.o:/home/denis/devprojects/ai-workspace/tests/test_providerregistry.cpp:58: first defined here
```

### Root Cause

- Had both test files in a single executable
- Each test file uses QTEST_MAIN() which expands to a main() function
- This caused multiple main() definitions in the same executable

### Fix Applied

Split tests into separate executables in CMakeLists.txt:

**Before** (single executable):
```cmake
qt_add_executable(tests
    test_providerregistry.cpp
    test_settingsmanager.cpp
)
```

**After** (separate executables):
```cmake
qt_add_executable(test_providerregistry
    test_providerregistry.cpp
    ../src/providers/Provider.cpp
    ../src/providers/ProviderRegistry.cpp
)

qt_add_executable(test_settingsmanager
    test_settingsmanager.cpp
    ../src/core/SettingsManager.cpp
)
```

### Lessons Learned

- QTEST_MAIN() macro expands to a main() function
- Can't have multiple test files with QTEST_MAIN in same executable
- Each test needs its own executable or use a custom test runner
- Must link required source files for each test executable

### Prevention

- Use separate executables for each test class
- Or use a custom main() that runs multiple test objects
- Document the chosen approach in CMakeLists.txt
- Consider using test suite pattern for related tests

---

## Mistake 5: Undefined References in Tests (Stage 1)

**Date**: 2026-10-10
**Stage**: Stage 1 - Project Foundation
**Severity**: Linker error

### Error

```
/usr/bin/ld: CMakeFiles/tests.dir/test_providerregistry.cpp.o: in function `TestProviderRegistry::testInstance()':
undefined reference to `ProviderRegistry::instance()'
undefined reference to `ProviderRegistry::providers() const'
undefined reference to `ProviderRegistry::providerById(QString const&) const'
```

### Root Cause

- Test executables didn't link the actual implementation files
- Only had the header files included
- Linker couldn't find the function implementations

### Fix Applied

Add source files to each test executable in CMakeLists.txt:

```cmake
qt_add_executable(test_providerregistry
    test_providerregistry.cpp
    ../src/providers/Provider.cpp        # Added
    ../src/providers/ProviderRegistry.cpp # Added
)
```

### Lessons Learned

- Headers only provide declarations, not implementations
- Must link .cpp files that contain the function implementations
- Each test executable needs its own copy of linked sources
- Consider using a static library for shared code

### Prevention

- Always link implementation files (.cpp) along with headers
- Create a static library for shared code used by multiple targets
- Verify linker can find all required symbols
- Use CMake's target_link_libraries correctly

---

## Mistake 6: Qt WebEngine devel Not Required for Skeleton (Stage 1)

**Date**: 2026-10-10
**Stage**: Stage 1 - Project Foundation
**Severity**: Planning/B blocker

### Issue

- Initial plan stated qt6-qtwebengine-devel must be installed before building
- This was incorrect for Stage 1 (application skeleton)
- Qt WebEngine is only needed for Stage 4 (browser integration)

### Fix Applied

1. Updated README.md to clarify qt6-qtwebengine-devel is not needed for Stage 1
2. Updated documentation to specify when Qt WebEngine is actually required
3. Proceeded with build without Qt WebEngine, which succeeded

### Lessons Learned

- Only install dependencies when actually needed for the current stage
- Separate build dependencies from runtime dependencies
- Document which stages require which dependencies
- Build incrementally to verify each stage works independently

### Prevention

- Document dependency requirements per stage
- Install dependencies as needed during implementation
- Keep Stage 1 minimal (no Qt WebEngine needed)
- Update documentation when dependency requirements change

---

---

## Mistake 7: Duplicate Destructor Definition (Stage 2)

**Date**: 2026-10-14
**Stage**: Stage 2 - Browser Engine, Profile Manager, Lifecycle State Machine
**Severity**: Build failure

### Error

```
/home/denis/devprojects/ai-workspace/src/lifecycle/LifecycleManager.cpp:12:1: error: definition of explicitly-defaulted 'LifecycleManager::~LifecycleManager()'
   12 | LifecycleManager::~LifecycleManager() = default;
      | ^~~~~~~~~~~~~~~~
In file included from /home/denis/devprojects/ai-workspace/src/lifecycle/LifecycleManager.cpp:1:
/home/denis/devprojects/ai-workspace/src/lifecycle/LifecycleManager.h:33:3: note: 'virtual LifecycleManager::~LifecycleManager()'
explicitly defaulted here
   33 |   ~LifecycleManager() override = default;
      |   ^
```

### Root Cause

- Destructor was defined as `= default` in both the header (.h) and the implementation (.cpp) file
- This is not allowed in C++ - explicitly defaulted functions should be defined in only one place
- The header already had `~LifecycleManager() override = default;`
- The .cpp file also had `LifecycleManager::~LifecycleManager() = default;`

### Fix Applied

Removed the duplicate destructor definition from LifecycleManager.cpp:

**Before**:
```cpp
LifecycleManager::LifecycleManager(QObject* parent)
  : QObject(parent)
  , m_freezeDelay(5 * 60 * 1000)
  , m_hibernateDelay(30 * 60 * 1000) {
}

LifecycleManager::~LifecycleManager() = default;

LifecycleManager* LifecycleManager::instance() {
```

**After**:
```cpp
LifecycleManager::LifecycleManager(QObject* parent)
  : QObject(parent)
  , m_freezeDelay(5 * 60 * 1000)
  , m_hibernateDelay(30 * 60 * 1000) {
}

LifecycleManager* LifecycleManager::instance() {
```

### Lessons Learned

- Explicitly defaulted functions (`= default`) should be defined in only one place
- For virtual destructors, it's usually better to define them in the header with `= default`
- If a function is defaulted in the header, do not also define it in the .cpp file
- This applies to both constructors and destructors

### Prevention

- When defaulting a function in the header, do not add it to the .cpp file
- When defaulting a function in the .cpp file, do not declare it with `= default` in the header
- Prefer defaulting in the header for inline destructors and simple constructors
- For out-of-line definitions, declare in header without `= default`, define in .cpp with `= default`

---

## Prevention Checklist

Before adding new code or configuration:

- [ ] Check Qt documentation for API existence in target version (Qt 6.10.3)
- [ ] Verify include directories are configured in CMakeLists.txt
- [ ] Check that source files include correct headers
- [ ] Verify linker will find all required implementations
- [ ] Use separate test executables or custom test runner
- [ ] Install only dependencies needed for current stage
- [ ] Test build from clean directory
- [ ] Run tests after build
- [ ] Document any new patterns or conventions learned
- [ ] Ensure defaulted functions are defined in only one place (header or .cpp, not both)

## Notes

- Qt 6.10.3 is the target version
- Always reference Qt 6.10.3 documentation, not generic Qt docs
- Google C++ style with C++20 standard
- CMake 3.31+ required
- Use INI format for QSettings (cross-platform consistency)
