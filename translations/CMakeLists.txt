file(GLOB TS_FILES *.ts)
set(TRANSLATION_SRC_DIR
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/src
    ${PROJECT_SOURCE_DIR}/test
    ${PROJECT_SOURCE_DIR}/tools)

# ts更新
execute_process(COMMAND lupdate-qt5 -no-obsolete ${TRANSLATION_SRC_DIR} -ts ${TS_FILES})

# qm生成,默认构建依赖
qt5_add_translation(QM_FILES ${TS_FILES})
add_custom_target(translations_qm ALL
    DEPENDS ${QM_FILES}
    COMMENT "Generating QM translation files"
)

install(FILES ${QM_FILES} DESTINATION ${TRANSLATIONS_INSTALL_DIR})