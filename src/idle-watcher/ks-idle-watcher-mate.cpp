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

#include "ks-idle-watcher-mate.h"
#include <qt5-log-i.h>
#include <QMap>

///NOTE:
/// 该枚举定义在mate-session-manager-1.22.1 gsm-presence.h中
/// 由于未提供开发头文件，故将其拷贝定义在此处
enum PresenceStatus
{
    PRESENCE_STATUS_AVAILABLE = 0,
    PRESENCE_STATUS_INVISIBLE,
    PRESENCE_STATUS_BUSY,
    PRESENCE_STATUS_IDLE
};

/// 用于打印空闲状态枚举值
static const QMap<PresenceStatus,QString> PresenceStatusDescMap = {
    {PRESENCE_STATUS_AVAILABLE,"available"},
    {PRESENCE_STATUS_INVISIBLE,"invisible"},
    {PRESENCE_STATUS_BUSY,"busy"},
    {PRESENCE_STATUS_IDLE,"idle"}
};

KSIdleWatcherMate::KSIdleWatcherMate(QObject *parent)
    : KSIdleWatcher(parent)
{
}

KSIdleWatcherMate::~KSIdleWatcherMate()
{
}

bool KSIdleWatcherMate::init()
{
    // 连接到session bus
    QDBusConnection sessionBus = QDBusConnection::sessionBus();
    if (!sessionBus.isConnected())
    {
        KLOG_ERROR() << "can't connect to session bus!";
        return false;
    }

    // 连接到SessionManager的DBus服务，处理会话状态改变
    delete m_presenceInterface;
    m_presenceInterface = nullptr;
    m_presenceInterface = new GSMPresenceProxy("org.gnome.SessionManager", "/org/gnome/SessionManager/Presence", sessionBus, this);
    if (!connect(m_presenceInterface, &GSMPresenceProxy::StatusChanged, this, &KSIdleWatcherMate::slotPresenceStatusChanged))
    {
        KLOG_ERROR() << "can't connect status changed signal!";
        return false;
    }
    if (!connect(m_presenceInterface, &GSMPresenceProxy::StatusTextChanged, this, &KSIdleWatcherMate::slotPresenceStatusTextChanged))
    {
        KLOG_ERROR() << "can't connect status text changed signal!";
        return false;
    }

    return true;
}

bool KSIdleWatcherMate::getIdleDetectionActive()
{
    return m_idleDetectionActive;
}

bool KSIdleWatcherMate::setIdleDetectionActive(bool idleDetectionActive)
{
    KLOG_DEBUG() << "turning idle watcher: " << (idleDetectionActive ?"ON":"OFF");

    if(idleDetectionActive == m_idleDetectionActive)
    {
        KLOG_DEBUG() << "idle watcher is already" << (idleDetectionActive ?"idleDetectionActive":"inactive");
        return false;
    }

    if( !m_enabled )
    {
        KLOG_DEBUG() << "idle watcher is disabled,cannot idleDetectionActive";
        return false;
    }

    m_idle = false;
    m_idleNotice = false;
    if(m_idleTimerID != 0)
    {
        killTimer(m_idleTimerID);
    }
    m_idleDetectionActive = idleDetectionActive;
    return true;
}

bool KSIdleWatcherMate::getEnabled()
{
    return m_enabled;
}

bool KSIdleWatcherMate::setEnabled(bool enabled)
{
    if(enabled == m_enabled)
    {
        KLOG_DEBUG() << "idle watcher is already" << (enabled?"enabled":"disabled");
        return false;
    }

    bool isActive = getIdleDetectionActive();
    if( !enabled && isActive )
    {
        setIdleDetectionActive(false);
    }

    m_enabled = enabled;
    return true;
}

void KSIdleWatcherMate::slotPresenceStatusChanged(uint status)
{
    auto iter  = PresenceStatusDescMap.find((PresenceStatus)status);
    KLOG_DEBUG() << "presence status changed: " << (iter==PresenceStatusDescMap.end()?"null":iter.value());
    setStatus(status);
}

void KSIdleWatcherMate::slotPresenceStatusTextChanged(const QString &statusText)
{
    KLOG_DEBUG() << "presence status text changed: " << statusText;
    m_statusMsg = statusText;
}

void KSIdleWatcherMate::setStatus(uint status)
{
    bool isIdle = false;

    if (!m_idleDetectionActive)
    {
        KLOG_DEBUG() << "not active,ignoring status changes";
        return;
    }

    isIdle = (status == PRESENCE_STATUS_IDLE);

    if (!isIdle && !m_idleNotice)
    {
        /*如果当前已经是处于空闲状态，并且已过了空闲预告的阶段则应该忽略掉该次空闲状态改变*/
        /* no change in idleness */
        return;     
    }

    if(m_idleTimerID)
    {
        // 若已存在空闲定时器，则销毁
        killTimer(m_idleTimerID);
        m_idleTimerID = 0;
    }

    if (isIdle)
    {
        // 设置空闲预告
        if( !setIdleNotice(true) )
        {
            return;
        }

        // 开启空闲定时器
        m_idleTimerID = startTimer(m_delayIdleTimeout);
    }
    else
    {
        if(m_idleTimerID!=0)
        {
            killTimer(m_idleTimerID);
            m_idleTimerID = 0;
        }
        setIdle(false);
        setIdleNotice(false);
    }
}

bool KSIdleWatcherMate::setIdleNotice(bool notice)
{
    bool  res = false;

    if(notice == m_idleNotice)
    {
        KLOG_DEBUG() << "idle notice no changes,ignore it";
        return res;
    }

    emit KSIdleWatcher::idleNoticeChanged(notice,res);

    if(Q_LIKELY(res))
    {
        KLOG_DEBUG() << "change idle notice state: " << notice;
        m_idleNotice = notice;
    }
    else
    {
        KLOG_DEBUG() << "idle notice changed signal not handled:" << res;
    }

    return res;
}

bool KSIdleWatcherMate::setIdle(bool idle)
{
    bool res = false;

    if(idle == m_idle)
    {
        KLOG_DEBUG() << "idle no changes,ignore it";
        return res;
    }

    emit KSIdleWatcher::idleChanged(idle,res);

    if(Q_LIKELY(res))
    {
        KLOG_DEBUG() << "change idle state: " << idle;
        m_idle = idle;
    }
    else
    {
        KLOG_DEBUG() << "idle changed signal not handled:" << res;
    }

    return res;
}

void KSIdleWatcherMate::timerEvent(QTimerEvent *event)
{
    if(event->timerId() == m_idleTimerID)
    {
        slotHandleIdleDelayTimeout();
    }
    QObject::timerEvent(event);
}

void KSIdleWatcherMate::slotHandleIdleDelayTimeout()
{
    bool res;

    res = setIdle(true);
    setIdleNotice(false);

    ///更新空闲状态成功，取消掉定时器，不再重复调用
    ///失败的话，重复调用
    if( res )
    {
        killTimer(m_idleTimerID);
        m_idleTimerID = 0;
    }
}
