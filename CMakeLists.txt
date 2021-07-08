cmake_minimum_required(VERSION 3.5)

project(kiran_screensaver)

set(CMAKE_CXX_STANDARD 11)

find_package(PkgConfig REQUIRED)
find_package(Qt5 COMPONENTS Core Gui Widgets DBus X11Extras REQUIRED)
pkg_search_module(KIRAN_LOG_QT5 REQUIRED klog-qt5)
pkg_search_module(KIRANWIDGETS_QT5 REQUIRED kiranwidgets-qt5)
pkg_search_module(GSETTINGS_QT REQUIRED gsettings-qt)
pkg_search_module(XCB REQUIRED xcb)
pkg_search_module(XCB_RANDR REQUIRED xcb-randr)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

file(GLOB_RECURSE SRC "src/*.cpp" "src/*.h")

#生成DBus服务适配器
set(DBUS_ADAPTOR_SRC "")
qt5_add_dbus_adaptor(DBUS_ADAPTOR_SRC
        data/org.mate.ScreenSaver.xml
        ${CMAKE_SOURCE_DIR}/src/ks-listener.h
        KSListener
        ks_mate_adaptor
        KSMateAdaptor)
qt5_add_dbus_adaptor(DBUS_ADAPTOR_SRC
        data/com.kylinsec.Kiran.ScreenSaver.xml
        ${CMAKE_SOURCE_DIR}/src/ks-listener.h
        KSListener
        ks_kiran_adaptor
        KSKiranAdaptor)
foreach(adaptor_item ${DBUS_ADAPTOR_SRC})
    set_property(SOURCE ${adaptor_item} PROPERTY SKIP_AUTOGEN ON)
endforeach()

#生成使用的DBus接口源文件
set(DBUS_INTERFACE_SRC "")
qt5_add_dbus_interface(DBUS_INTERFACE_SRC data/org.gnome.SessionManager.Presence.xml gnome_session_presence)
foreach(interface_item ${DBUS_INTERFACE_SRC})
    set_property(SOURCE ${interface_item} PROPERTY SKIP_AUTOGEN ON)
endforeach()

add_executable(${PROJECT_NAME}
        ${SRC}
        ${DBUS_ADAPTOR_SRC}
        ${DBUS_INTERFACE_SRC})

include_directories(${CMAKE_BINARY_DIR}
        src/fade
        src/idle-watcher
        src/tools
        src/listener
        src/grab
        src/view)

target_link_libraries(${PROJECT_NAME}
        Qt5::Core
        Qt5::Gui
        Qt5::Widgets
        Qt5::DBus
        Qt5::X11Extras
        ${KIRAN_LOG_QT5_LIBRARIES}
        ${KIRANWIDGETS_QT5_LIBRARIES}
        ${GSETTINGS_QT_LIBRARIES}
        ${XCB_LIBRARIES}
        ${XCB_RANDR_LIBRARIES})

target_include_directories(${PROJECT_NAME} PRIVATE
        ${KIRAN_LOG_QT5_INCLUDE_DIRS}
        ${KIRANWIDGETS_QT5_INCLUDE_DIRS}
        ${GSETTINGS_QT_INCLUDE_DIRS}
        ${XCB_INCLUDE_DIRS}
        ${XCB_RANDR_LIBRARIES})

add_subdirectory(test)


