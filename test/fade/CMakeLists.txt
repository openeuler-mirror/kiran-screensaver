cmake_minimum_required(VERSION 3.5)

set(TARGET_NAME "test-fade")

find_package(PkgConfig REQUIRED)
find_package(Qt5 COMPONENTS Core Gui Widgets DBus X11Extras REQUIRED)
pkg_search_module(KIRAN_LOG_QT5 REQUIRED klog-qt5)
pkg_search_module(XCB REQUIRED xcb)
pkg_search_module(XCB_RANDR REQUIRED xcb-randr)

file(GLOB_RECURSE TEST_FADE_H_FILES ./*.h)
file(GLOB_RECURSE TEST_FADE_CPP_FILES ./*.cpp)

file(GLOB_RECURSE FADE_SRC "../../src/fade/*.cpp" "../../src/fade/*.h")
file(GLOB_RECURSE TOOLS_SRC "../../src/tools/*.cpp" "../../src/tools/*.h")

add_executable(${TARGET_NAME}
               ${FADE_SRC}
               ${TOOLS_SRC}
               ${TEST_FADE_H_FILES}
               ${TEST_FADE_CPP_FILES})

target_link_libraries(${TARGET_NAME}
        Qt5::Core
        Qt5::DBus
        Qt5::Gui
        Qt5::Widgets
        Qt5::X11Extras
        ${KIRAN_LOG_QT5_LIBRARIES}
        ${XCB_LIBRARIES}
        ${XCB_RANDR_LIBRARIES})

target_include_directories(${TARGET_NAME} PRIVATE
        ../../src/tools/
        ../../src/fade/
        ${KIRAN_LOG_QT5_INCLUDE_DIRS}
        ${XCB_LIBRARIES}
        ${XCB_RANDR_LIBRARIES})