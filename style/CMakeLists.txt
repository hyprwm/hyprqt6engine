project(hypr-style)

set(app_SRCS
  plugin.cpp
  ProxyStyle.cpp
)

add_library(hypr-style MODULE ${app_SRCS})
target_link_libraries(hypr-style PRIVATE Qt6::Widgets hyprqt6engine-common hyprlang hyprutils)
install(TARGETS hypr-style DESTINATION ${PLUGINDIR}/styles)
