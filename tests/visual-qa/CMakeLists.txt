# Sizing diagnostic test
qt_add_executable(sizing-diagnostic
  sizing-diagnostic.cpp
)

target_link_libraries(sizing-diagnostic PRIVATE
  ai-workspace-lib
  Qt6::Core
  Qt6::Widgets
  Qt6::WebEngineWidgets
)

target_include_directories(sizing-diagnostic PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/../../src/core
  ${CMAKE_CURRENT_SOURCE_DIR}/../../src/ui
  ${CMAKE_CURRENT_SOURCE_DIR}/../../src/providers
  ${CMAKE_CURRENT_SOURCE_DIR}/../../src/profiles
  ${CMAKE_CURRENT_SOURCE_DIR}/../../src/lifecycle
  ${CMAKE_CURRENT_SOURCE_DIR}/../../src
)

# State launcher - launches app in specific UI state for desktop screenshots
qt_add_executable(state-launcher
  state-launcher.cpp
)

target_link_libraries(state-launcher PRIVATE
  ai-workspace-lib
  Qt6::Core
  Qt6::Widgets
  Qt6::WebEngineWidgets
)

target_include_directories(state-launcher PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/../../src/core
  ${CMAKE_CURRENT_SOURCE_DIR}/../../src/ui
  ${CMAKE_CURRENT_SOURCE_DIR}/../../src/providers
  ${CMAKE_CURRENT_SOURCE_DIR}/../../src/profiles
  ${CMAKE_CURRENT_SOURCE_DIR}/../../src/lifecycle
  ${CMAKE_CURRENT_SOURCE_DIR}/../../src
)
