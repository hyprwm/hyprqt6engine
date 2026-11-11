project(hyprqtplugin)

set(app_SRCS
  main.cpp
  PlatformTheme.cpp
  # helpers/MiscFunctions.cpp
)

add_library(hyprqt6engine MODULE ${app_SRCS})
set_target_properties(hyprqt6engine PROPERTIES OUTPUT_NAME hyprqt6engine)
target_link_libraries(hyprqt6engine PRIVATE $<TARGET_NAME_IF_EXISTS:Qt6::WidgetsPrivate> Qt6::GuiPrivate $<TARGET_NAME_IF_EXISTS:Qt6::QuickControls2> $<TARGET_NAME_IF_EXISTS:KF6::IconThemes> hyprutils hyprlang hyprqt6engine-common)
target_compile_definitions(hyprqt6engine PRIVATE $<$<TARGET_EXISTS:KF6::IconThemes>:KF_ICONTHEMES_LIB> $<$<TARGET_EXISTS:KF6::ConfigCore>:KF_CONFIGCORE_LIB> $<$<TARGET_EXISTS:KF6::ColorScheme>:KF_COLORSCHEME_LIB>)
install(TARGETS hyprqt6engine DESTINATION ${PLUGINDIR}/platformthemes)
