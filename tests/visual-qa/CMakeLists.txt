# Visual QA harness - captures screenshots of the application in various states
qt_add_executable(visual-qa
  visual-qa.cpp
)

target_link_libraries(visual-qa PRIVATE
  ai-workspace-lib
  Qt6::Core
  Qt6::Test
  Qt6::Widgets
  Qt6::WebEngineWidgets
)

target_include_directories(visual-qa PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/../../src/core
  ${CMAKE_CURRENT_SOURCE_DIR}/../../src/ui
  ${CMAKE_CURRENT_SOURCE_DIR}/../../src/providers
  ${CMAKE_CURRENT_SOURCE_DIR}/../../src/profiles
  ${CMAKE_CURRENT_SOURCE_DIR}/../../src/lifecycle
  ${CMAKE_CURRENT_SOURCE_DIR}/../../src
)
