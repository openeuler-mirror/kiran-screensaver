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
#include "dbus-session.h"
#include <QDebug>
#include <QProcess>
#include <QTest>

QProcess* DBusSession::dbusSessionProcess = nullptr;

bool DBusSession::initDBusSession()
{
    if (DBusSession::dbusSessionProcess)
    {
        return true;
    }

    QScopedPointer<DBusSession> process(new DBusSession());
    process->start("/usr/bin/dbus-daemon", QStringList() << "--print-address"
                                                         << "--fork"
                                                         << "--session",
                   QIODevice::ReadOnly);
    process->waitForStarted();
    if (process->state() != QProcess::Running)
    {
        qCritical() << "Failed to start dbus-daemon" << process->errorString();
        return false;
    }

    dbusSessionProcess = process.take();
    auto dbusAddress = dbusSessionProcess->readAllStandardOutput().trimmed();
    qAddPostRoutine(+[]()
                    {
                        if (dbusSessionProcess)
                        {
                            dbusSessionProcess->kill();
                            dbusSessionProcess = nullptr;
                        }
                    });

    qputenv("DBUS_SESSION_BUS_ADDRESS", dbusAddress);
    qputenv("XDG_CURRENT_DESKTOP", "KIRAN");
    return true;
}