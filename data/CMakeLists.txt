configure_file(config.h.in ${CMAKE_BINARY_DIR}/config.h @ONLY)
configure_file(com.kylinsec.Kiran.ScreenSaver.service.in ${CMAKE_BINARY_DIR}/com.kylinsec.Kiran.ScreenSaver.service @ONLY)
configure_file(com.kylinsec.kiran.screensaver.gschema.xml.in ${CMAKE_BINARY_DIR}/com.kylinsec.kiran.screensaver.gschema.xml )

#不安装dbus-1 service文件
#1. 若安装dbus-1service文件,kiran-session-daemon POWER插件可能过早拉起kiran-screensaver导致获取壁纸失败
#2. 若安装dbus-1service文件,systemd拉起kiran-screensaver可能出现通过Logind Manager的GetSessionByPID方法获取当前logind session dbus地址失败的问题
#经过考虑,kiran-screensaver只安装xdg/autostart文件,通过会话管理拉起
#install(FILES ${CMAKE_BINARY_DIR}/com.kylinsec.Kiran.ScreenSaver.service DESTINATION ${DBUS_SERVICE_INSTALL_DIR})
install(FILES kiran-screensaver.desktop DESTINATION ${XDG_AUTO_START_DIR})
install(FILES kiran-screensaver DESTINATION /${CMAKE_INSTALL_SYSCONFDIR}/pam.d/)
install(FILES ${CMAKE_BINARY_DIR}/com.kylinsec.kiran.screensaver.gschema.xml DESTINATION ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_DATADIR}/glib-2.0/schemas/)
install(FILES kiran-screensaver.ini DESTINATION /${CMAKE_INSTALL_SYSCONFDIR}/)