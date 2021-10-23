cmake_minimum_required(VERSION 3.5)

project(kiran-screensaver)

set(CMAKE_CXX_STANDARD 11)

find_package(PkgConfig REQUIRED)
find_package(Qt5 COMPONENTS Core Gui Widgets DBus X11Extras LinguistTools REQUIRED)
pkg_search_module(KIRAN_LOG_QT5 REQUIRED klog-qt5)
pkg_search_module(KIRANWIDGETS_QT5 REQUIRED kiranwidgets-qt5)
pkg_search_module(GSETTINGS_QT REQUIRED gsettings-qt)
pkg_search_module(XCB REQUIRED xcb)
pkg_search_module(XCB_RANDR REQUIRED xcb-randr)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

#生成DBus服务适配器
set(DBUS_ADAPTOR_SRC "")
qt5_add_dbus_adaptor(DBUS_ADAPTOR_SRC
        data/org.mate.ScreenSaver.xml
        ${CMAKE_SOURCE_DIR}/src/listener/ks-listener.h
        KSListener
        ks_mate_adaptor
        KSMateAdaptor)
qt5_add_dbus_adaptor(DBUS_ADAPTOR_SRC
        data/com.kylinsec.Kiran.ScreenSaver.xml
        ${CMAKE_SOURCE_DIR}/src/listener/ks-listener.h
        KSListener
        ks_kiran_adaptor
        KSKiranAdaptor)
foreach(adaptor_item ${DBUS_ADAPTOR_SRC})
    set_property(SOURCE ${adaptor_item} PROPERTY SKIP_AUTOGEN ON)
endforeach()

#生成使用的DBus接口源文件
set(GSM_PRESENCE_XML data/org.gnome.SessionManager.Presence.xml)
set_source_files_properties(${GSM_PRESENCE_XML}
        PROPERTIES
        CLASSNAME GSMPresenceProxy
        NO_NAMESPACE true)
qt5_add_dbus_interface(GSM_PRESENCE_SRC ${GSM_PRESENCE_XML} gsm_presence_proxy)
foreach(interface_item ${GSM_PRESENCE_SRC})
    set_property(SOURCE ${interface_item} PROPERTY SKIP_AUTOGEN ON)
endforeach()

file(GLOB_RECURSE SRC "src/*.cpp" "src/*.h")
file(GLOB DEVEL_HEADER "include/*.h")
#翻译文件
set( TS_FILES "translations/kiran-screensaver.zh_CN.ts" )
qt5_create_translation( QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES} )

add_executable(${PROJECT_NAME}
        ${DBUS_ADAPTOR_SRC}
        ${GSM_PRESENCE_SRC}
        ${SRC}
        ${DEVEL_HEADER}
        ${QM_FILES}
        resources/kiran-screensaver.qrc)

target_include_directories(${PROJECT_NAME} PRIVATE
        ${CMAKE_BINARY_DIR}
        src
        src/fade
        src/idle-watcher
        src/tools
        src/listener
        src/grab
        src/view
        src/screensaver
        src/locker
        src/widgets

        include

        ${KIRAN_LOG_QT5_INCLUDE_DIRS}
        ${KIRANWIDGETS_QT5_INCLUDE_DIRS}
        ${GSETTINGS_QT_INCLUDE_DIRS}
        ${XCB_INCLUDE_DIRS}
        ${XCB_RANDR_LIBRARIES})

target_link_libraries(${PROJECT_NAME} PUBLIC
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

include(GNUInstallDirs)

set(SCREENSAVER_BIN_DIR ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_BINDIR})
set(DBUS_SERVICE_INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_DATADIR}/dbus-1/services)
set(XDG_AUTO_START_DIR /etc/xdg/autostart)
set(TRANSLATIONS_INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_DATADIR}/kiran-screensaver/translations/)

add_subdirectory(tools)

configure_file(data/config.h.in ${CMAKE_BINARY_DIR}/config.h @ONLY)
configure_file(data/org.mate.ScreenSaver.service.in ${CMAKE_BINARY_DIR}/org.mate.ScreenSaver.service @ONLY)
configure_file(data/com.kylinsec.Kiran.ScreenSaver.service.in ${CMAKE_BINARY_DIR}/com.kylinsec.Kiran.ScreenSaver.service)

install(TARGETS ${PROJECT_NAME} DESTINATION ${SCREENSAVER_BIN_DIR})
install(FILES ${QM_FILES} DESTINATION ${TRANSLATIONS_INSTALL_DIR} )
install(FILES ${DEVEL_HEADER} DESTINATION ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_INCLUDEDIR}/kiran-screensaver)
install(FILES ${CMAKE_BINARY_DIR}/org.mate.ScreenSaver.service DESTINATION ${DBUS_SERVICE_INSTALL_DIR})
install(FILES ${CMAKE_BINARY_DIR}/com.kylinsec.Kiran.ScreenSaver.service DESTINATION ${DBUS_SERVICE_INSTALL_DIR})
install(FILES data/kiran-screensaver.desktop DESTINATION ${XDG_AUTO_START_DIR})
install(FILES data/kiran-screensaver DESTINATION /${CMAKE_INSTALL_SYSCONFDIR}/pam.d/ )
install(FILES data/com.kylinsec.kiran.screensaver.gschema.xml DESTINATION ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_DATADIR}/glib-2.0/schemas/ )
