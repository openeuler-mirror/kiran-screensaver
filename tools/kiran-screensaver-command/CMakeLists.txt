set(TARGET_NAME "kiran-screensaver-command")

add_executable(${TARGET_NAME} main.cpp)

target_link_libraries(${TARGET_NAME} PRIVATE
        Qt5::Core
        Qt5::DBus)

install(TARGETS ${TARGET_NAME} DESTINATION ${SCREENSAVER_BIN_DIR})
