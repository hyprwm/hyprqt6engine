project(hyprqt6engine-common)

add_definitions(-DHYPRQT6ENGINE_LIBRARY)

set(app_SRCS
  common.cpp
  config/ConfigManager.cpp
)

add_library(hyprqt6engine-common SHARED ${app_SRCS})
target_link_libraries(hyprqt6engine-common PRIVATE Qt6::Gui $<TARGET_NAME_IF_EXISTS:KF6::ConfigCore> $<TARGET_NAME_IF_EXISTS:KF6::ColorScheme> hyprutils hyprlang)
target_compile_definitions(hyprqt6engine-common PRIVATE $<$<TARGET_EXISTS:KF6::ConfigCore>:KF_CONFIGCORE_LIB> $<$<TARGET_EXISTS:KF6::ColorScheme>:KF_COLORSCHEME_LIB>)
install(TARGETS hyprqt6engine-common DESTINATION ${CMAKE_INSTALL_LIBDIR})
