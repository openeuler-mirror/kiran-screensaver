enable_testing()

set(TARGET_NAME kiran-screensaver-test)

# 测试代码
file(GLOB_RECURSE TEST_SRCS "./*.cpp" "./*.h")

kiran_qt5_add_dbus_interface_ex(
  DBUS_SRC ${PROJECT_SOURCE_DIR}/dbus/com.kylinsec.Kiran.ScreenSaver.xml
  kiran_screensaver_proxy KiranScreenSaverProxy)
list(APPEND TEST_SRCS ${DBUS_SRC})

# 主程序代码
file(GLOB_RECURSE SCREENSAVER_SRC "${PROJECT_SOURCE_DIR}/src/*.cpp"
     "${PROJECT_SOURCE_DIR}/src/*.h")
list(REMOVE_ITEM SCREENSAVER_SRC ${PROJECT_SOURCE_DIR}/src/main.cpp)

qt5_add_dbus_adaptor(
  DBUS_SRC ${PROJECT_SOURCE_DIR}/dbus/com.kylinsec.Kiran.ScreenSaver.xml
  ${PROJECT_SOURCE_DIR}/src/listener/listener.h Kiran::ScreenSaver::Listener
  kiran_adaptor KiranAdaptor)
list(APPEND SCREENSAVER_SRC ${DBUS_SRC})

kiran_qt5_add_dbus_interface_ex(
  DBUS_SRC ${PROJECT_SOURCE_DIR}/dbus/org.gnome.SessionManager.Presence.xml
  gsm_presence_proxy GSMPresenceProxy)
list(APPEND SCREENSAVER_SRC ${DBUS_SRC})

kiran_qt5_add_dbus_interface_ex(
  DBUS_SRC
  ${PROJECT_SOURCE_DIR}/dbus/com.kylinsec.Kiran.SessionDaemon.Appearance.xml
  kiran_appearance_proxy KiranAppearance)
list(APPEND SCREENSAVER_SRC ${DBUS_SRC})

add_executable(${TARGET_NAME} ${TEST_SRCS} ${SCREENSAVER_SRC})
target_link_libraries(
  ${TARGET_NAME}
  Qt5::Core
  Qt5::Gui
  Qt5::Widgets
  Qt5::DBus
  Qt5::Gui
  Qt5::Test
  ${KIRANWIDGETS_QT5_LIBRARIES}
  ${KIRAN_LOG_QT5_LIBRARIES}
  ${XCB_LIBRARIES}
  ${XCB_RANDR_LIBRARIES})

target_include_directories(
  ${TARGET_NAME}
  PRIVATE ${PROJECT_BINARY_DIR}/
          ${PROJECT_BINARY_DIR}/test
          ${PROJECT_BINARY_DIR}/data
          ${PROJECT_SOURCE_DIR}/include
          ${PROJECT_SOURCE_DIR}/src
          ${SRC_INTERNAL_DIRS}
          ${KIRANWIDGETS_QT5_INCLUDE_DIRS}
          ${KIRAN_LOG_QT5_INCLUDE_DIRS}
          ${XCB_INCLUDE_DIRS}
          ${XCB_RANDR_INCLUDE_DIRS})

add_test(NAME ${TARGET_NAME} COMMAND ${TARGET_NAME})
