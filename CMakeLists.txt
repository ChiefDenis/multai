cmake_minimum_required(VERSION 3.31)
project(ai-workspace VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets)

set(PROJECT_SOURCES
    src/core/main.cpp
    src/core/Application.cpp
    src/core/SettingsManager.cpp
    src/providers/Provider.cpp
    src/providers/ProviderRegistry.cpp
    src/ui/MainWindow.cpp
    src/ui/TabWidget.cpp
    src/ui/ProviderSelectionDialog.cpp
)

set(PROJECT_HEADERS
    src/core/Application.h
    src/core/SettingsManager.h
    src/providers/Provider.h
    src/providers/ProviderRegistry.h
    src/ui/MainWindow.h
    src/ui/TabWidget.h
    src/ui/ProviderSelectionDialog.h
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
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(ai-workspace PRIVATE
    Qt6::Core
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

enable_testing()

add_subdirectory(tests)
