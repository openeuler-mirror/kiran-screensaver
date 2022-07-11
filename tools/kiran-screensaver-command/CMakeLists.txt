set(TARGET_NAME "kiran-screensaver-command")
set( CMD_TS_FILES "translations/kiran-screensaver-command.zh_CN.ts" )
qt5_create_translation( CMD_QM_FILES ${CMAKE_CURRENT_SOURCE_DIR} ${CMD_TS_FILES} )

add_executable(${TARGET_NAME} main.cpp ${CMD_QM_FILES})

target_link_libraries(${TARGET_NAME} PRIVATE
        Qt5::Core
        Qt5::DBus)

install(TARGETS ${TARGET_NAME} DESTINATION ${SCREENSAVER_BIN_DIR})
install(FILES ${CMD_QM_FILES} DESTINATION ${TRANSLATIONS_INSTALL_DIR})