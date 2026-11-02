cmake_minimum_required(VERSION 3.31)
project(ai-workspace VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets WebEngineWidgets)

set(PROJECT_SOURCES
    src/core/main.cpp
    src/core/Application.cpp
    src/core/SettingsManager.cpp
    src/core/TabStore.cpp
    src/providers/Provider.cpp
    src/providers/ProviderRegistry.cpp
    src/providers/ProviderIcons.cpp
    src/ui/MainWindow.cpp
    src/ui/TabWidget.cpp
    src/ui/ProviderSelectionDialog.cpp
    src/ui/ProfileSelectionDialog.cpp
    src/ui/ProfileManagementDialog.cpp
    src/ui/SettingsDialog.cpp
    src/profiles/ProfileManager.cpp
    src/profiles/ProfileStorage.cpp
    src/profiles/Profile.cpp
    src/lifecycle/TabMetadata.cpp
    src/lifecycle/BrowserTab.cpp
    src/lifecycle/BrowserView.cpp
    src/lifecycle/LifecycleManager.cpp
    src/lifecycle/MemoryMonitor.cpp
    src/lifecycle/TestClock.cpp
)

set(PROJECT_HEADERS
    src/core/Application.h
    src/core/SettingsManager.h
    src/core/TabStore.h
    src/providers/Provider.h
    src/providers/ProviderRegistry.h
    src/providers/ProviderIcons.h
    src/ui/MainWindow.h
    src/ui/TabWidget.h
    src/ui/ProviderSelectionDialog.h
    src/ui/ProfileSelectionDialog.h
    src/ui/ProfileManagementDialog.h
    src/ui/SettingsDialog.h
    src/profiles/ProfileManager.h
    src/profiles/ProfileStorage.h
    src/profiles/Profile.h
    src/lifecycle/TabMetadata.h
    src/lifecycle/LifecycleTarget.h
    src/lifecycle/BrowserTab.h
    src/lifecycle/BrowserView.h
    src/lifecycle/LifecycleManager.h
    src/lifecycle/MemoryMonitor.h
    src/lifecycle/TestClock.h
)

qt_add_executable(ai-workspace
    ${PROJECT_SOURCES}
    ${PROJECT_HEADERS}
)

qt_add_resources(ai-workspace "resources"
    PREFIX "/"
    FILES
)

target_include_directories(ai-workspace PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ui
    ${CMAKE_CURRENT_SOURCE_DIR}/src/providers
    ${CMAKE_CURRENT_SOURCE_DIR}/src/profiles
    ${CMAKE_CURRENT_SOURCE_DIR}/src/lifecycle
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(ai-workspace PRIVATE
    Qt6::Core
    Qt6::Widgets
    Qt6::WebEngineWidgets
)

set_target_properties(ai-workspace PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)

include(GNUInstallDirs)

install(TARGETS ai-workspace
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

enable_testing()

add_subdirectory(tests)
