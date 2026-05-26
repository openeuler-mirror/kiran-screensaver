file(GLOB TS_FILES *.ts)
set(TRANSLATION_SRC_DIR
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/src
    ${PROJECT_SOURCE_DIR}/test
    ${PROJECT_SOURCE_DIR}/tools)

# qt_create_translation会导致cmake clean把ts文件删掉
# qt5_add_translation 5.9.4以下生成的qm文件不带${LANG}后缀
# https://bugreports.qt.io/browse/QTBUG-64317

# 定义支持的语言列表
set(LANGUAGES zh_CN) # 可以添加更多语言  # set(LANGUAGES zh_CN bo_CN ug_CN mn_MN ky_KG
                     # kk_KZ)

set(QM_FILES "")
foreach(LANG ${LANGUAGES})
        set(TS_FILE "${PROJECT_SOURCE_DIR}/translations/${PROJECT_NAME}.${LANG}.ts")
        execute_process(COMMAND lupdate-qt5 -no-obsolete ${TRANSLATION_SRC_DIR} -ts ${TS_FILE})
        set(QM_FILE "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.${LANG}.qm")
        execute_process(COMMAND lrelease-qt5 ${TS_FILE} -qm ${QM_FILE})
        list(APPEND QM_FILES ${QM_FILE})
        message(STATUS "QM_FILE: ${QM_FILE}")
endforeach()

install(FILES ${QM_FILES} DESTINATION ${TRANSLATIONS_INSTALL_DIR})