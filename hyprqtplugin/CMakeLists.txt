project(hyprqtplugin)

set(app_SRCS
  main.cpp
  PlatformTheme.cpp
  helpers/MiscFunctions.cpp
)

add_library(hyprqtplugin MODULE ${app_SRCS})
set_target_properties(hyprqtplugin PROPERTIES OUTPUT_NAME hyprqt6engine)
target_link_libraries(hyprqtplugin PRIVATE $<TARGET_NAME_IF_EXISTS:Qt6::WidgetsPrivate> Qt6::GuiPrivate $<TARGET_NAME_IF_EXISTS:Qt6::QuickControls2> $<TARGET_NAME_IF_EXISTS:KF6::IconThemes> hyprutils hyprlang hyprqt6engine-common)
target_compile_definitions(hyprqtplugin PRIVATE $<$<TARGET_EXISTS:KF6::IconThemes>:KF_ICONTHEMES_LIB>)
install(TARGETS hyprqtplugin DESTINATION ${PLUGINDIR}/platformthemes)
