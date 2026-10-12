find_package(Qt6 REQUIRED COMPONENTS Test)

# ProviderRegistry test
qt_add_executable(test_providerregistry
  test_providerregistry.cpp
  ../src/providers/Provider.cpp
  ../src/providers/ProviderRegistry.cpp
)

target_link_libraries(test_providerregistry PRIVATE
  Qt6::Core
  Qt6::Test
  Qt6::Widgets
)

target_include_directories(test_providerregistry PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/../src/core
  ${CMAKE_CURRENT_SOURCE_DIR}/../src/ui
  ${CMAKE_CURRENT_SOURCE_DIR}/../src/providers
  ${CMAKE_CURRENT_SOURCE_DIR}/../src
)

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

target_include_directories(test_settingsmanager PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/../src/core
  ${CMAKE_CURRENT_SOURCE_DIR}/../src/ui
  ${CMAKE_CURRENT_SOURCE_DIR}/../src/providers
  ${CMAKE_CURRENT_SOURCE_DIR}/../src
)

add_test(NAME test_settingsmanager COMMAND test_settingsmanager)
