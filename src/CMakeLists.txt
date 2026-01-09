file(GLOB_RECURSE SRC "./*.cpp" "./*.h" "./*.ui")

qt5_add_dbus_adaptor(
  DBUS_SRC ${PROJECT_SOURCE_DIR}/dbus/com.kylinsec.Kiran.ScreenSaver.xml
  ${PROJECT_SOURCE_DIR}/src/listener/listener.h Kiran::ScreenSaver::Listener
  kiran_adaptor KiranAdaptor)
list(APPEND SRC ${DBUS_SRC})

kiran_qt5_add_dbus_interface_ex(
  DBUS_SRC ${PROJECT_SOURCE_DIR}/dbus/org.gnome.SessionManager.Presence.xml
  gsm_presence_proxy GSMPresenceProxy)
list(APPEND SRC ${DBUS_SRC})

kiran_qt5_add_dbus_interface_ex(
  DBUS_SRC
  ${PROJECT_SOURCE_DIR}/dbus/com.kylinsec.Kiran.SessionDaemon.Appearance.xml
  kiran_appearance_proxy KiranAppearance)
list(APPEND SRC ${DBUS_SRC})

add_executable(
  ${PROJECT_NAME} ${SRC} ${DEVEL_HEADER}
                  ${PROJECT_SOURCE_DIR}/resources/kiran-screensaver.qrc)

# 获取当前目录下的所有子目录，用于头文件包含
file(GLOB SRC_SUBFILES ${CMAKE_CURRENT_SOURCE_DIR}/*)

set(SRC_INTERNAL_DIRS "")
foreach(subfile ${SRC_SUBFILES})
  if(IS_DIRECTORY ${subfile})
    list(APPEND SRC_INTERNAL_DIRS ${subfile})
  endif()
endforeach()

set(SRC_INTERNAL_DIRS
    ${SRC_INTERNAL_DIRS}
    PARENT_SCOPE)

target_include_directories(
  ${PROJECT_NAME}
  PRIVATE ${CMAKE_BINARY_DIR}
          ${CMAKE_BINARY_DIR}/src
          ${PROJECT_SOURCE_DIR}/include
          ${CMAKE_CURRENT_SOURCE_DIR}
          ${SRC_INTERNAL_DIRS}
          ${KIRAN_LOG_QT5_INCLUDE_DIRS}
          ${KIRANWIDGETS_QT5_INCLUDE_DIRS}
          ${GSETTINGS_QT_INCLUDE_DIRS}
          ${XCB_INCLUDE_DIRS}
          ${XCB_RANDR_INCLUDE_DIRS})

target_link_libraries(
  ${PROJECT_NAME}
  PUBLIC Qt5::Core
         Qt5::Gui
         Qt5::Widgets
         Qt5::DBus
         Qt5::X11Extras
         ${KIRAN_LOG_QT5_LIBRARIES}
         ${KIRANWIDGETS_QT5_LIBRARIES}
         ${GSETTINGS_QT_LIBRARIES}
         ${XCB_LIBRARIES}
         ${XCB_RANDR_LIBRARIES})

install(TARGETS ${PROJECT_NAME} DESTINATION ${SCREENSAVER_BIN_DIR})
