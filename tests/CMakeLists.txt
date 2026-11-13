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
  ../src/providers/Provider.cpp
  ../src/providers/ProviderRegistry.cpp
  ../src/providers/ProviderIcons.cpp
)

target_link_libraries(test_tabstore PRIVATE
  Qt6::Core
  Qt6::Test
  Qt6::Widgets
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
  ../src/providers/Provider.cpp
  ../src/providers/ProviderRegistry.cpp
  ../src/providers/ProviderIcons.cpp
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
  ../src/ui/ToolbarIcons.cpp
  ../src/providers/Provider.cpp
  ../src/providers/ProviderRegistry.cpp
  ../src/providers/ProviderIcons.cpp
  ../src/core/SettingsManager.cpp
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
  ../src/ui/ToolbarIcons.cpp
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
  ../src/providers/Provider.cpp
  ../src/providers/ProviderRegistry.cpp
  ../src/providers/ProviderIcons.cpp
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

# Lifecycle Integration test
qt_add_executable(test_lifecycle_integration
  test_lifecycle_integration.cpp
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
  ../src/providers/Provider.cpp
  ../src/providers/ProviderRegistry.cpp
  ../src/providers/ProviderIcons.cpp
)

target_link_libraries(test_lifecycle_integration PRIVATE
  Qt6::Core
  Qt6::Test
  Qt6::Widgets
  Qt6::WebEngineWidgets
)

target_include_directories(test_lifecycle_integration PRIVATE ${TEST_INCLUDE_DIRS})

add_test(NAME test_lifecycle_integration COMMAND test_lifecycle_integration)
set_property(TEST test_lifecycle_integration PROPERTY LABELS "integration")

# Profile Isolation test
qt_add_executable(test_profile_isolation
  test_profile_isolation.cpp
  ../src/profiles/Profile.cpp
  ../src/profiles/ProfileManager.cpp
  ../src/profiles/ProfileStorage.cpp
  ../src/core/SettingsManager.cpp
)

target_link_libraries(test_profile_isolation PRIVATE
  Qt6::Core
  Qt6::Test
  Qt6::Widgets
  Qt6::WebEngineWidgets
)

target_include_directories(test_profile_isolation PRIVATE ${TEST_INCLUDE_DIRS})

add_test(NAME test_profile_isolation COMMAND test_profile_isolation)
set_property(TEST test_profile_isolation PROPERTY LABELS "integration")

# Persistence test
qt_add_executable(test_persistence
  test_persistence.cpp
  ../src/core/TabStore.cpp
  ../src/core/SettingsManager.cpp
  ../src/lifecycle/TabMetadata.cpp
  ../src/providers/Provider.cpp
  ../src/providers/ProviderRegistry.cpp
  ../src/providers/ProviderIcons.cpp
)

target_link_libraries(test_persistence PRIVATE
  Qt6::Core
  Qt6::Test
  Qt6::Widgets
)

target_include_directories(test_persistence PRIVATE ${TEST_INCLUDE_DIRS})

add_test(NAME test_persistence COMMAND test_persistence)
set_property(TEST test_persistence PROPERTY LABELS "integration")

# Browser Integration test
qt_add_executable(test_browser_integration
  test_browser_integration.cpp
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
  ../src/providers/Provider.cpp
  ../src/providers/ProviderRegistry.cpp
  ../src/providers/ProviderIcons.cpp
)

target_link_libraries(test_browser_integration PRIVATE
  Qt6::Core
  Qt6::Test
  Qt6::Widgets
  Qt6::WebEngineWidgets
)

target_include_directories(test_browser_integration PRIVATE ${TEST_INCLUDE_DIRS})

add_test(NAME test_browser_integration COMMAND test_browser_integration)
set_property(TEST test_browser_integration PROPERTY LABELS "integration")

# Memory Usage Performance test
qt_add_executable(test_memory_usage
  test_memory_usage.cpp
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
  ../src/providers/Provider.cpp
  ../src/providers/ProviderRegistry.cpp
  ../src/providers/ProviderIcons.cpp
)

target_link_libraries(test_memory_usage PRIVATE
  Qt6::Core
  Qt6::Test
  Qt6::Widgets
  Qt6::WebEngineWidgets
)

target_include_directories(test_memory_usage PRIVATE ${TEST_INCLUDE_DIRS})

add_test(NAME test_memory_usage COMMAND test_memory_usage)
set_property(TEST test_memory_usage PROPERTY LABELS "performance")

# Tab Timings Performance test
qt_add_executable(test_tab_timings
  test_tab_timings.cpp
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
  ../src/providers/Provider.cpp
  ../src/providers/ProviderRegistry.cpp
  ../src/providers/ProviderIcons.cpp
)

target_link_libraries(test_tab_timings PRIVATE
  Qt6::Core
  Qt6::Test
  Qt6::Widgets
  Qt6::WebEngineWidgets
)

target_include_directories(test_tab_timings PRIVATE ${TEST_INCLUDE_DIRS})

add_test(NAME test_tab_timings COMMAND test_tab_timings)
set_property(TEST test_tab_timings PROPERTY LABELS "performance")

# Profile Growth Performance test
qt_add_executable(test_profile_growth
  test_profile_growth.cpp
  ../src/profiles/Profile.cpp
  ../src/profiles/ProfileManager.cpp
  ../src/profiles/ProfileStorage.cpp
  ../src/core/SettingsManager.cpp
)

target_link_libraries(test_profile_growth PRIVATE
  Qt6::Core
  Qt6::Test
  Qt6::Widgets
  Qt6::WebEngineWidgets
)

target_include_directories(test_profile_growth PRIVATE ${TEST_INCLUDE_DIRS})

add_test(NAME test_profile_growth COMMAND test_profile_growth)
set_property(TEST test_profile_growth PROPERTY LABELS "performance")

# Auto Default Profile test
qt_add_executable(test_auto_default_profile
  test_auto_default_profile.cpp
  ../src/profiles/Profile.cpp
  ../src/profiles/ProfileManager.cpp
  ../src/profiles/ProfileStorage.cpp
  ../src/core/SettingsManager.cpp
  ../src/lifecycle/BrowserTab.cpp
  ../src/lifecycle/BrowserView.cpp
  ../src/lifecycle/LifecycleManager.cpp
  ../src/lifecycle/MemoryMonitor.cpp
  ../src/lifecycle/TabMetadata.cpp
  ../src/lifecycle/TestClock.cpp
  ../src/providers/Provider.cpp
  ../src/providers/ProviderRegistry.cpp
  ../src/providers/ProviderIcons.cpp
)

target_link_libraries(test_auto_default_profile PRIVATE
  Qt6::Core
  Qt6::Test
  Qt6::Widgets
  Qt6::WebEngineWidgets
)

target_include_directories(test_auto_default_profile PRIVATE ${TEST_INCLUDE_DIRS})

add_test(NAME test_auto_default_profile COMMAND test_auto_default_profile)
set_property(TEST test_auto_default_profile PROPERTY LABELS "integration")

# Empty State test
qt_add_executable(test_empty_state
  test_empty_state.cpp
  ../src/ui/BrowserWorkspace.cpp
  ../src/ui/ToolbarIcons.cpp
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
  ../src/providers/Provider.cpp
  ../src/providers/ProviderRegistry.cpp
  ../src/providers/ProviderIcons.cpp
)

target_link_libraries(test_empty_state PRIVATE
  Qt6::Core
  Qt6::Test
  Qt6::Widgets
  Qt6::WebEngineWidgets
)

target_include_directories(test_empty_state PRIVATE ${TEST_INCLUDE_DIRS})

add_test(NAME test_empty_state COMMAND test_empty_state)
set_property(TEST test_empty_state PROPERTY LABELS "integration")

# Sidebar resize regression test
qt_add_executable(test_sidebar_resize
  test_sidebar_resize.cpp
)

target_link_libraries(test_sidebar_resize PRIVATE
  ai-workspace-lib
  Qt6::Core
  Qt6::Test
  Qt6::Widgets
  Qt6::WebEngineWidgets
)

target_include_directories(test_sidebar_resize PRIVATE ${TEST_INCLUDE_DIRS})

add_test(NAME test_sidebar_resize COMMAND test_sidebar_resize)
set_property(TEST test_sidebar_resize PROPERTY LABELS "integration")

# Empty state UI regression test
qt_add_executable(test_empty_state_ui
  test_empty_state_ui.cpp
)

target_link_libraries(test_empty_state_ui PRIVATE
  ai-workspace-lib
  Qt6::Core
  Qt6::Test
  Qt6::Widgets
  Qt6::WebEngineWidgets
)

target_include_directories(test_empty_state_ui PRIVATE ${TEST_INCLUDE_DIRS})

add_test(NAME test_empty_state_ui COMMAND test_empty_state_ui)
set_property(TEST test_empty_state_ui PROPERTY LABELS "integration")

# UI Regression test - dark theme, sidebar, empty state, header icons
qt_add_executable(test_ui_regression
  test_ui_regression.cpp
)

target_link_libraries(test_ui_regression PRIVATE
  ai-workspace-lib
  Qt6::Core
  Qt6::Test
  Qt6::Widgets
  Qt6::WebEngineWidgets
)

target_include_directories(test_ui_regression PRIVATE ${TEST_INCLUDE_DIRS})

add_test(NAME test_ui_regression COMMAND test_ui_regression)
set_property(TEST test_ui_regression PROPERTY LABELS "integration")

# Visual corrections regression test - header, toolbar icons, lifecycle consistency,
# dark surfaces, collapsed navigation, capture-state configuration
qt_add_executable(test_visual_corrections
  test_visual_corrections.cpp
)

target_link_libraries(test_visual_corrections PRIVATE
  ai-workspace-lib
  Qt6::Core
  Qt6::Test
  Qt6::Widgets
  Qt6::WebEngineWidgets
)

target_include_directories(test_visual_corrections PRIVATE ${TEST_INCLUDE_DIRS})

add_test(NAME test_visual_corrections COMMAND test_visual_corrections)
set_property(TEST test_visual_corrections PROPERTY LABELS "integration")

# Visual QA harness - not registered as ctest test (manual QA tool)
add_subdirectory(visual-qa)

# Session persistence and restoration test
qt_add_executable(test_session_restore
  test_session_restore.cpp
)

target_link_libraries(test_session_restore PRIVATE
  ai-workspace-lib
  Qt6::Core
  Qt6::Test
  Qt6::Widgets
  Qt6::WebEngineWidgets
)

target_include_directories(test_session_restore PRIVATE ${TEST_INCLUDE_DIRS})

add_test(NAME test_session_restore COMMAND test_session_restore)
set_property(TEST test_session_restore PROPERTY LABELS "integration")

# Provider logo assets and navigation rendering regression test
qt_add_executable(test_provider_logos
  test_provider_logos.cpp
)

target_link_libraries(test_provider_logos PRIVATE
  ai-workspace-lib
  Qt6::Core
  Qt6::Test
  Qt6::Widgets
  Qt6::WebEngineWidgets
)

target_include_directories(test_provider_logos PRIVATE ${TEST_INCLUDE_DIRS})

add_test(NAME test_provider_logos COMMAND test_provider_logos)
set_property(TEST test_provider_logos PROPERTY LABELS "integration")

# Tests that compile ProviderIcons.cpp directly need the bundled SVG resources.
set(PROVIDER_LOGO_TEST_TARGETS
  test_providerregistry
  test_tabstore
  test_recovery
  test_ui_polish
  test_sidebar
  test_browserworkspace
  test_lifecycle_integration
  test_persistence
  test_browser_integration
  test_memory_usage
  test_tab_timings
  test_auto_default_profile
  test_empty_state
)

foreach(target IN LISTS PROVIDER_LOGO_TEST_TARGETS)
  target_link_libraries(${target} PRIVATE provider-logos)
endforeach()
