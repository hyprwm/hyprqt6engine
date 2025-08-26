cmake_minimum_required(VERSION 3.16)

# Get version
file(READ "${CMAKE_SOURCE_DIR}/VERSION" VER_RAW)
string(STRIP ${VER_RAW} VER)

project(hyprqt6engine VERSION ${VER} LANGUAGES CXX)

set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 6.9 CONFIG REQUIRED COMPONENTS BuildInternals Core Widgets)
find_package(PkgConfig REQUIRED)
find_package(KF6Config)
find_package(KF6ColorScheme)
find_package(KF6IconThemes)

get_target_property(QT_QTPATHS_EXECUTABLE Qt6::qtpaths IMPORTED_LOCATION)

if(NOT PLUGINDIR)
    #execute_process(COMMAND ${QT_QTPATHS_EXECUTABLE} -query QT_INSTALL_PLUGINS OUTPUT_VARIABLE PLUGINDIR OUTPUT_STRIP_TRAILING_WHITESPACE)
    execute_process(COMMAND ${QT_QTPATHS_EXECUTABLE} --plugin-dir OUTPUT_VARIABLE PLUGINDIR OUTPUT_STRIP_TRAILING_WHITESPACE)
endif()

if(PLUGINDIR)
   message(STATUS "Plugin path: ${PLUGINDIR}")
else()
   message(FATAL_ERROR "Could not get plugin path")
endif()

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/common)

add_definitions(-DQT_DISABLE_DEPRECATED_BEFORE=0x060200 -DQT_DEPRECATED_WARNINGS -DQT_MESSAGELOGCONTEXT)
add_compile_options(-Wall -Wextra)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_BUILD_RPATH_USE_ORIGIN ON)

pkg_check_modules(
  deps
  REQUIRED
  IMPORTED_TARGET
  hyprutils
  hyprlang
)

add_subdirectory(hyprqtplugin)
add_subdirectory(common)
add_subdirectory(style)

qt_standard_project_setup(REQUIRES 6.9)
