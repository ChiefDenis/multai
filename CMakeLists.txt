cmake_minimum_required(VERSION 3.31)

option(BUILD_TESTS "Build tests" ON)

project(ai-workspace VERSION 0.4.3 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Generate version header from CMake project version
configure_file(src/core/version.h.in src/core/version.h @ONLY)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets WebEngineWidgets)

set(LIB_SOURCES
    src/core/Application.cpp
    src/core/SettingsManager.cpp
    src/core/TabStore.cpp
    src/providers/Provider.cpp
    src/providers/ProviderRegistry.cpp
    src/providers/ProviderIcons.cpp
    src/ui/MainWindow.cpp
    src/ui/Sidebar.cpp
    src/ui/BrowserWorkspace.cpp
    src/ui/ProviderSelectionDialog.cpp
    src/ui/ProfileSelectionDialog.cpp
    src/ui/ProfileManagementDialog.cpp
    src/ui/SettingsDialog.cpp
    src/ui/Theme.cpp
    src/ui/ToolbarIcons.cpp
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

qt_add_library(provider-logos STATIC
    resources/ProviderLogos.cpp
)

qt_add_resources(provider-logos "providerlogos"
    PREFIX "/provider-logos"
    BASE "assets/providers"
    FILES
        assets/providers/chatgpt.svg
        assets/providers/claude.svg
        assets/providers/copilot.svg
        assets/providers/deepseek.svg
        assets/providers/gemini.svg
        assets/providers/glm.svg
        assets/providers/kimi.svg
        assets/providers/perplexity.svg
        assets/providers/qwen.svg
)

target_link_libraries(provider-logos PRIVATE Qt6::Core)

set(PROJECT_HEADERS
    src/core/Application.h
    src/core/SettingsManager.h
    src/core/TabStore.h
    src/providers/Provider.h
    src/providers/ProviderRegistry.h
    src/providers/ProviderIcons.h
    src/ui/MainWindow.h
    src/ui/Sidebar.h
    src/ui/BrowserWorkspace.h
    src/ui/ProviderSelectionDialog.h
    src/ui/ProfileSelectionDialog.h
    src/ui/ProfileManagementDialog.h
    src/ui/SettingsDialog.h
    src/ui/Theme.h
    src/ui/ToolbarIcons.h
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

qt_add_library(ai-workspace-lib STATIC
    ${LIB_SOURCES}
    ${PROJECT_HEADERS}
)

qt_add_resources(ai-workspace-lib "appicon"
    PREFIX "/icons"
    BASE "icons"
    FILES icons/hicolor/256x256/apps/ai-workspace.png
)

target_include_directories(ai-workspace-lib PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR}/src/core
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ui
    ${CMAKE_CURRENT_SOURCE_DIR}/src/providers
    ${CMAKE_CURRENT_SOURCE_DIR}/src/profiles
    ${CMAKE_CURRENT_SOURCE_DIR}/src/lifecycle
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(ai-workspace-lib PRIVATE
    Qt6::Core
    Qt6::Widgets
    Qt6::WebEngineWidgets
    provider-logos
)

qt_add_executable(ai-workspace
    src/core/main.cpp
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
    ai-workspace-lib
    Qt6::Widgets
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

install(FILES ai-workspace.desktop
    DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/applications
)

# Install icons
install(FILES icons/hicolor/16x16/apps/ai-workspace.png
    DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor/16x16/apps
    RENAME ai-workspace.png
)
install(FILES icons/hicolor/24x24/apps/ai-workspace.png
    DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor/24x24/apps
    RENAME ai-workspace.png
)
install(FILES icons/hicolor/32x32/apps/ai-workspace.png
    DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor/32x32/apps
    RENAME ai-workspace.png
)
install(FILES icons/hicolor/48x48/apps/ai-workspace.png
    DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor/48x48/apps
    RENAME ai-workspace.png
)
install(FILES icons/hicolor/64x64/apps/ai-workspace.png
    DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor/64x64/apps
    RENAME ai-workspace.png
)
install(FILES icons/hicolor/128x128/apps/ai-workspace.png
    DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor/128x128/apps
    RENAME ai-workspace.png
)
install(FILES icons/hicolor/256x256/apps/ai-workspace.png
    DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor/256x256/apps
    RENAME ai-workspace.png
)
install(FILES icons/hicolor/scalable/apps/ai-workspace.svg
    DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor/scalable/apps
    RENAME ai-workspace.svg
)

if(BUILD_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif()
