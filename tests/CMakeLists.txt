find_package(Qt6 REQUIRED COMPONENTS Test WebEngineWidgets)

set(TEST_INCLUDE_DIRS
  ${CMAKE_CURRENT_SOURCE_DIR}/../src/core
  ${CMAKE_CURRENT_SOURCE_DIR}/../src/ui
  ${CMAKE_CURRENT_SOURCE_DIR}/../src/providers
  ${CMAKE_CURRENT_SOURCE_DIR}/../src/profiles
  ${CMAKE_CURRENT_SOURCE_DIR}/../src/lifecycle
  ${CMAKE_CURRENT_SOURCE_DIR}/../src
)

# ProviderRegistry test
qt_add_executable(test_providerregistry
  test_providerregistry.cpp
  ../src/providers/Provider.cpp
  ../src/providers/ProviderRegistry.cpp
  ../src/providers/ProviderIcons.cpp
)

target_link_libraries(test_providerregistry PRIVATE
  Qt6::Core
  Qt6::Test
  Qt6::Widgets
)

target_include_directories(test_providerregistry PRIVATE ${TEST_INCLUDE_DIRS})

add_test(NAME test_providerregistry COMMAND test_providerregistry)

# SettingsManager test
qt_add_executable(test_settingsmanager
  test_settingsmanager.cpp
  ../src/core/SettingsManager.cpp
)

target_link_libraries(test_settingsmanager PRIVATE
  Qt6::Core
  Qt6::Test
  Qt6::Widgets
)

target_include_directories(test_settingsmanager PRIVATE ${TEST_INCLUDE_DIRS})

add_test(NAME test_settingsmanager COMMAND test_settingsmanager)

# Lifecycle state machine test
qt_add_executable(test_lifecycle
  test_lifecycle.cpp
  ../src/lifecycle/TabMetadata.cpp
)

target_link_libraries(test_lifecycle PRIVATE
  Qt6::Core
  Qt6::Test
)

target_include_directories(test_lifecycle PRIVATE ${TEST_INCLUDE_DIRS})

add_test(NAME test_lifecycle COMMAND test_lifecycle)

# LifecycleManager test
qt_add_executable(test_lifecyclemanager
  test_lifecyclemanager.cpp
  ../src/lifecycle/LifecycleManager.cpp
  ../src/lifecycle/MemoryMonitor.cpp
  ../src/lifecycle/TabMetadata.cpp
  ../src/lifecycle/TestClock.cpp
  ../src/core/SettingsManager.cpp
)

target_link_libraries(test_lifecyclemanager PRIVATE
  Qt6::Core
  Qt6::Test
)

target_include_directories(test_lifecyclemanager PRIVATE ${TEST_INCLUDE_DIRS})

add_test(NAME test_lifecyclemanager COMMAND test_lifecyclemanager)

# MemoryMonitor test
qt_add_executable(test_memorymonitor
  test_memorymonitor.cpp
  ../src/lifecycle/MemoryMonitor.cpp
  ../src/core/SettingsManager.cpp
)

target_link_libraries(test_memorymonitor PRIVATE
  Qt6::Core
  Qt6::Test
)

target_include_directories(test_memorymonitor PRIVATE ${TEST_INCLUDE_DIRS})

add_test(NAME test_memorymonitor COMMAND test_memorymonitor)

# TabStore test
qt_add_executable(test_tabstore
  test_tabstore.cpp
  ../src/core/TabStore.cpp
  ../src/core/SettingsManager.cpp
  ../src/lifecycle/TabMetadata.cpp
)

target_link_libraries(test_tabstore PRIVATE
  Qt6::Core
  Qt6::Test
)

target_include_directories(test_tabstore PRIVATE ${TEST_INCLUDE_DIRS})

add_test(NAME test_tabstore COMMAND test_tabstore)

# ProfileManager test
qt_add_executable(test_profilemanager
  test_profilemanager.cpp
  ../src/profiles/Profile.cpp
  ../src/profiles/ProfileManager.cpp
  ../src/profiles/ProfileStorage.cpp
  ../src/core/SettingsManager.cpp
)

target_link_libraries(test_profilemanager PRIVATE
  Qt6::Core
  Qt6::Test
  Qt6::Widgets
  Qt6::WebEngineWidgets
)

target_include_directories(test_profilemanager PRIVATE ${TEST_INCLUDE_DIRS})

add_test(NAME test_profilemanager COMMAND test_profilemanager)

# Error Recovery test
qt_add_executable(test_recovery
  test_recovery.cpp
  ../src/lifecycle/BrowserTab.cpp
  ../src/lifecycle/BrowserView.cpp
  ../src/lifecycle/LifecycleManager.cpp
  ../src/lifecycle/MemoryMonitor.cpp
  ../src/lifecycle/TabMetadata.cpp
  ../src/lifecycle/TestClock.cpp
  ../src/profiles/Profile.cpp
  ../src/profiles/ProfileManager.cpp
  ../src/profiles/ProfileStorage.cpp
  ../src/core/SettingsManager.cpp
)

target_link_libraries(test_recovery PRIVATE
  Qt6::Core
  Qt6::Test
  Qt6::Widgets
  Qt6::WebEngineWidgets
)

target_include_directories(test_recovery PRIVATE ${TEST_INCLUDE_DIRS})

add_test(NAME test_recovery COMMAND test_recovery)

# UI Polish & Theming test
qt_add_executable(test_ui_polish
  test_ui_polish.cpp
  ../src/ui/Theme.cpp
  ../src/providers/ProviderIcons.cpp
)

target_link_libraries(test_ui_polish PRIVATE
  Qt6::Core
  Qt6::Test
  Qt6::Widgets
)

target_include_directories(test_ui_polish PRIVATE ${TEST_INCLUDE_DIRS})

add_test(NAME test_ui_polish COMMAND test_ui_polish)

# Sidebar test
qt_add_executable(test_sidebar
  test_sidebar.cpp
  ../src/ui/Sidebar.cpp
  ../src/providers/Provider.cpp
  ../src/providers/ProviderRegistry.cpp
  ../src/providers/ProviderIcons.cpp
)

target_link_libraries(test_sidebar PRIVATE
  Qt6::Core
  Qt6::Test
  Qt6::Widgets
  Qt6::WebEngineWidgets
)

target_include_directories(test_sidebar PRIVATE ${TEST_INCLUDE_DIRS})

add_test(NAME test_sidebar COMMAND test_sidebar)

# BrowserWorkspace test
qt_add_executable(test_browserworkspace
  test_browserworkspace.cpp
  ../src/ui/BrowserWorkspace.cpp
  ../src/lifecycle/BrowserTab.cpp
  ../src/lifecycle/BrowserView.cpp
  ../src/lifecycle/LifecycleManager.cpp
  ../src/lifecycle/MemoryMonitor.cpp
  ../src/lifecycle/TabMetadata.cpp
  ../src/lifecycle/TestClock.cpp
  ../src/profiles/Profile.cpp
  ../src/profiles/ProfileManager.cpp
  ../src/profiles/ProfileStorage.cpp
  ../src/core/SettingsManager.cpp
)

target_link_libraries(test_browserworkspace PRIVATE
  Qt6::Core
  Qt6::Test
  Qt6::Widgets
  Qt6::WebEngineWidgets
)

target_include_directories(test_browserworkspace PRIVATE ${TEST_INCLUDE_DIRS})

add_test(NAME test_browserworkspace COMMAND test_browserworkspace)

# SettingsDialog test
qt_add_executable(test_settingsdialog
  test_settingsdialog.cpp
  ../src/ui/SettingsDialog.cpp
  ../src/core/SettingsManager.cpp
)

target_link_libraries(test_settingsdialog PRIVATE
  Qt6::Core
  Qt6::Test
  Qt6::Widgets
)

target_include_directories(test_settingsdialog PRIVATE ${TEST_INCLUDE_DIRS})

add_test(NAME test_settingsdialog COMMAND test_settingsdialog)
