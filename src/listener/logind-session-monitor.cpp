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
#include "logind-session-monitor.h"

#include <qt5-log-i.h>
#include <QCoreApplication>
#include <QDBusInterface>
#include <QDBusReply>

static const QString LOGIND_SERVICE = "org.freedesktop.login1";
static const QString LOGIND_PATH = "/org/freedesktop/login1";
static const QString LOGIND_MANAGER_INTERFACE = "org.freedesktop.login1.Manager";
static const QString LOGIND_SESSION_INTERFACE = "org.freedesktop.login1.Session";

LogindSessionMonitor::LogindSessionMonitor(QObject *parent) : QObject(parent)
{

}

LogindSessionMonitor::~LogindSessionMonitor()
{

}

bool LogindSessionMonitor::init()
{
    //NOTE:
    // 偶发出现过从logind取出logind当前的session路径，报错未能找到该PID属于的session
    // 这种情况下在开始菜单点开的终端里执行kiran-screensaver会出现
    // 在caja右键打开的终端里执行不会出现
    // 暂时未进行排查,是桌面环境哪个组件导致该问题发生

    //获取logind session dbus对象路径，用于监听dbus信号
    QDBusInterface loginInterface(LOGIND_SERVICE, LOGIND_PATH, LOGIND_MANAGER_INTERFACE, QDBusConnection::systemBus());
    QDBusReply<QDBusObjectPath> reply = loginInterface.call("GetSessionByPID", (uint32_t)0);
    if (!reply.isValid())
    {
        KLOG_ERROR() << "can't get session dbus path!" << reply.error();
        return false;
    }

    QString sessionPath = reply.value().path();
    if ( !QDBusConnection::systemBus().connect(LOGIND_SERVICE, sessionPath,
                                             LOGIND_SESSION_INTERFACE, "Lock",
                                             this, SIGNAL(Lock())) ||
        !QDBusConnection::systemBus().connect(LOGIND_SERVICE, sessionPath,
                                             LOGIND_SESSION_INTERFACE, "Unlock",
                                             this, SIGNAL(Unlock())))
    {
        KLOG_ERROR() << "can't connect logind session Lock/Unlock signal!" << QDBusConnection::systemBus().lastError();
        return false;
    }

    return true;
}
