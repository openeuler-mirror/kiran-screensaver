/**
 * Copyright (c) 2020 ~ 2021 KylinSec Co., Ltd.
 * kiran-screensaver is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 *
 * Author:     liuxinhao <liuxinhao@kylinos.com.cn>
 */

#include <kiran-application.h>
#include <QProcess>
#include <QTranslator>

#include "manager.h"
#include "qt5-log-i.h"

int main(int argc, char *argv[])
{
    klog_qt5_init("",
                  "kylinsec-session",
                  "kiran-screensaver",
                  "kiran-screensaver");

    // NOTE:
    // xsmp QueryEndSession 会话询问退出阶段
    // QxcbSessionManager -> QGuiApplicationPrivate::commitData将会尝试关闭所有窗口，导致进程退出(#15523)
    // 1. 由于依赖会话管理AutoRestart功能，需向会话管理注册，不能禁用SessionManager。
    // 2. 不能重写QWidget::closeEvent ignore事件，会影响整个会话流程。
    
    // 加入标志，影响流程 QGuiApplicationPrivate::commitData。不关闭所有窗口，也不影响会话管理AutoRestart功能
    QGuiApplication::setFallbackSessionManagementEnabled(false);

    // 窗口全部关闭时也不退出
    QGuiApplication::setQuitOnLastWindowClosed(false);

    KiranApplication app(argc, argv);

    int xsetProcess = QProcess::execute("xset", QStringList() << "s" << "0" << "0");
    KLOG_DEBUG() << "xset execute result:" << xsetProcess;

    auto translator = new QTranslator;
    if (translator->load(QLocale(),
                         "kiran-screensaver",
                         ".",
                         "/usr/share/kiran-screensaver/translations/",
                         ".qm"))
    {
        app.installTranslator(translator);
    }
    else
    {
        qWarning() << "can't load translator";
    }

    Kiran::ScreenSaver::Manager manager;
    if (!manager.init())
    {
        KLOG_ERROR() << "kiran-screensaver init failed! exit.";
        return EXIT_FAILURE;
    }

    return QApplication::exec();
}
