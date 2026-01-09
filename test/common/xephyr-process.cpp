/**
 * Copyright (c) 2020 ~ 2026 KylinSec Co., Ltd.
 * kiran-screensaver is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 *
 * Author:     liuxinhao <liuxinhao@kylinsec.com.cn>
 */
#include "xephyr-process.h"
#include <signal.h>
#include <unistd.h>
#include <QTest>

bool XephyrProcess::isXephyrReady = false;
XephyrProcess* XephyrProcess::xephyrProcess = nullptr;

#define XEPHYR_DISPLAY ":99"
#define XEPHYR_INIT_TIMEOUT 2000
#define XEPHYR_INIT_INTERVAL 100

XephyrProcess::~XephyrProcess()
{
    if (XephyrProcess::xephyrProcess)
    {
        XephyrProcess::xephyrProcess->kill();
    }
}

bool XephyrProcess::initXephyr()
{
    if (XephyrProcess::xephyrProcess)
    {
        return true;
    }

    // 注册SIGUSR1信号的处理函数,捕获Xorg进程发送的SIGUSR1信号
    struct sigaction action;
    action.sa_sigaction = +[](int signum, siginfo_t* info, void* context)
    {
        if (info->si_signo == SIGUSR1)
        {
            isXephyrReady = true;
        }
    };
    sigemptyset(&action.sa_mask);
    action.sa_flags = SA_SIGINFO | SA_RESTART;
    sigaction(SIGUSR1, &action, NULL);

    // 运行内嵌Xserver
    QScopedPointer<XephyrProcess> process(new XephyrProcess());
    process->start("/usr/bin/Xephyr", QStringList() << "-screen"
                                                    << "1024x768" << XEPHYR_DISPLAY << "-ac",
                   QIODevice::ReadOnly);
    process->waitForStarted();
    if (process->state() != QProcess::Running)
    {
        qCritical() << "Failed to start Xephyr" << process->errorString();
        return false;
    }

    // 等待Xserver初始化完成
    int timeout = 0;
    while (!isXephyrReady &&
           timeout < XEPHYR_INIT_TIMEOUT &&
           (timeout += XEPHYR_INIT_INTERVAL))
    {
        QTest::qWait(XEPHYR_INIT_INTERVAL);
    }

    // 解除SIGUSR1信号的处理函数注册，恢复默认行为
    struct sigaction default_action;
    default_action.sa_handler = SIG_DFL;
    sigemptyset(&default_action.sa_mask);
    default_action.sa_flags = 0;
    sigaction(SIGUSR1, &default_action, NULL);

    XephyrProcess::xephyrProcess = process.take();
    qAddPostRoutine(+[]()
                    {
                        if (XephyrProcess::xephyrProcess)
                        {
                            XephyrProcess::xephyrProcess->kill();
                            XephyrProcess::xephyrProcess = nullptr;
                        }
                    });

    qputenv("DISPLAY", XEPHYR_DISPLAY);
    return true;
}

void XephyrProcess::setupChildProcess()
{
    // 将SIGUSR1信号设置为SIG_IGN
    // 确保Xorg会执行NotifyParentProcess函数通知父进程初始化完毕
    signal(SIGUSR1, SIG_IGN);
}