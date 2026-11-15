cmake_minimum_required(VERSION 3.31)

option(BUILD_TESTS "Build tests" ON)

project(Multai VERSION 0.5.0 LANGUAGES CXX)

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
    src/core/DataMigration.cpp
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
    src/core/AppIdentity.h
    src/core/Application.h
    src/core/DataMigration.h
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

qt_add_library(multai-lib STATIC
    ${LIB_SOURCES}
    ${PROJECT_HEADERS}
)

qt_add_resources(multai-lib "appicon"
    PREFIX "/icons"
    BASE "icons"
    FILES
        icons/hicolor/256x256/apps/multai.png
)

target_include_directories(multai-lib PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR}/src/core
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ui
    ${CMAKE_CURRENT_SOURCE_DIR}/src/providers
    ${CMAKE_CURRENT_SOURCE_DIR}/src/profiles
    ${CMAKE_CURRENT_SOURCE_DIR}/src/lifecycle
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(multai-lib PRIVATE
    Qt6::Core
    Qt6::Widgets
    Qt6::WebEngineWidgets
    provider-logos
)

qt_add_executable(multai
    src/core/main.cpp
)

target_include_directories(multai PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ui
    ${CMAKE_CURRENT_SOURCE_DIR}/src/providers
    ${CMAKE_CURRENT_SOURCE_DIR}/src/profiles
    ${CMAKE_CURRENT_SOURCE_DIR}/src/lifecycle
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(multai PRIVATE
    multai-lib
    Qt6::Widgets
)

set_target_properties(multai PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)

include(GNUInstallDirs)

install(TARGETS multai
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(FILES multai.desktop
    DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/applications
)

# Install the canonical icon name. It must not contain '-' because KDE's
# KIconLoader strips hyphen-separated suffixes inside each theme before it
# consults hicolor, so a hyphenated name can resolve to an unrelated mimetype
# icon in themes such as Kora. "multai" is hyphen-free.
install(FILES icons/hicolor/16x16/apps/multai.png
    DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor/16x16/apps
)
install(FILES icons/hicolor/24x24/apps/multai.png
    DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor/24x24/apps
)
install(FILES icons/hicolor/32x32/apps/multai.png
    DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor/32x32/apps
)
install(FILES icons/hicolor/48x48/apps/multai.png
    DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor/48x48/apps
)
install(FILES icons/hicolor/64x64/apps/multai.png
    DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor/64x64/apps
)
install(FILES icons/hicolor/128x128/apps/multai.png
    DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor/128x128/apps
)
install(FILES icons/hicolor/256x256/apps/multai.png
    DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor/256x256/apps
)
install(FILES icons/hicolor/scalable/apps/multai.svg
    DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor/scalable/apps
)

if(BUILD_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif()
