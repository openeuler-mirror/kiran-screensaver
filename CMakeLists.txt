cmake_minimum_required(VERSION 3.5)

project(kiran-screensaver)

# 屏幕保护以及解锁窗口分开触发 适用于>=KiranUI2.6
option(SPLIT_SCREENSAVER_AND_LOCK "Split screensavers and lock windows" OFF)
if(SPLIT_SCREENSAVER_AND_LOCK)
  set(SPLIT_SCREENSAVER_AND_LOCK_DEF true)
else()
  set(SPLIT_SCREENSAVER_AND_LOCK_DEF false)
endif()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(qdbus-wrapper)
include(GNUInstallDirs)

find_package(PkgConfig REQUIRED)
find_package(
  Qt5
  COMPONENTS Core
             Gui
             Widgets
             DBus
             X11Extras
             LinguistTools
             Test
  REQUIRED)

pkg_search_module(KIRAN_LOG_QT5 REQUIRED klog-qt5)
pkg_search_module(KIRANWIDGETS_QT5 REQUIRED kiranwidgets-qt5)
pkg_search_module(GSETTINGS_QT REQUIRED gsettings-qt)
pkg_search_module(XCB REQUIRED xcb)
pkg_search_module(XCB_RANDR REQUIRED xcb-randr)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(XDG_AUTO_START_DIR /etc/xdg/autostart)
set(SCREENSAVER_BIN_DIR ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_BINDIR})
set(DBUS_SERVICE_INSTALL_DIR
    ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_DATADIR}/dbus-1/services)
set(SCREENSAVER_PLUGIN_DIR
    ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}/kiran-screensaver)
set(TRANSLATIONS_INSTALL_DIR
    ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_DATADIR}/kiran-screensaver/translations/
)

file(GLOB DEVEL_HEADER "include/*.h")
install(
  FILES ${DEVEL_HEADER}
  DESTINATION
    ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_INCLUDEDIR}/kiran-screensaver)

add_subdirectory(data)
add_subdirectory(translations)
add_subdirectory(src)
add_subdirectory(test)
add_subdirectory(tools)
