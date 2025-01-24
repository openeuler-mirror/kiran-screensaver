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

#include "idle-watcher-mate.h"
#include <qt5-log-i.h>
#include <QMap>

using namespace Kiran::ScreenSaver;

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

Q_LOGGING_CATEGORY(qLcIdleWatcher,"kiran.ss.idlewatcher",QtMsgType::QtDebugMsg)

IdleWatcherMate::IdleWatcherMate(QObject *parent)
    : IdleWatcher(parent)
{
}

IdleWatcherMate::~IdleWatcherMate()
{
}

bool IdleWatcherMate::init()
{
    // 连接到session bus
    QDBusConnection sessionBus = QDBusConnection::sessionBus();
    if (!sessionBus.isConnected())
    {
        KLOG_ERROR(qLcIdleWatcher) << "connect to session bus daemon failed";
        return false;
    }

    // 连接到SessionManager的DBus服务，处理会话状态改变
    delete m_presenceInterface;
    m_presenceInterface = nullptr;
    m_presenceInterface = new GSMPresenceProxy("org.gnome.SessionManager", "/org/gnome/SessionManager/Presence", sessionBus, this);
    if (!connect(m_presenceInterface, &GSMPresenceProxy::StatusChanged, this, &IdleWatcherMate::slotPresenceStatusChanged))
    {
        KLOG_ERROR(qLcIdleWatcher) << "connect to SessionManager for monitor session idle failed";
        return false;
    }
    if (!connect(m_presenceInterface, &GSMPresenceProxy::StatusTextChanged, this, &IdleWatcherMate::slotPresenceStatusTextChanged))
    {
        KLOG_ERROR(qLcIdleWatcher) << "connect to SessionManager StatustextChanged failed";
        return false;
    }

    KLOG_INFO(qLcIdleWatcher) << "SessionManager idle status is monitored";
    return true;
}

bool IdleWatcherMate::getIdleDetectionActive()
{
    return m_idleDetectionActive;
}

bool IdleWatcherMate::setIdleDetectionActive(bool idleDetectionActive)
{
    KLOG_INFO(qLcIdleWatcher) << "turning idle watcher: " << (idleDetectionActive ?"ON":"OFF");

    if(idleDetectionActive == m_idleDetectionActive)
    {
        KLOG_DEBUG(qLcIdleWatcher) << "idle watcher is already" << (idleDetectionActive ?"idleDetectionActive":"inactive");
        return false;
    }

    if( !m_enabled )
    {
        KLOG_WARNING(qLcIdleWatcher) << "idle watcher is disabled, set active status failed";
        return false;
    }

    m_idle = false;
    m_idleNotice = false;

    //NOTE:由于空闲预告阶段会抓取鼠标键盘到屏幕外,所以空闲预告阶段不会出现设置空闲检测的情况,暂时不需要考虑特殊情况
    if(m_idleTimerID != 0)
    {
        killTimer(m_idleTimerID);
        m_idleTimerID = 0;
    }

    m_idleDetectionActive = idleDetectionActive;
    return true;
}

bool IdleWatcherMate::getEnabled()
{
    return m_enabled;
}

bool IdleWatcherMate::setEnabled(bool enabled)
{
    if(enabled == m_enabled)
    {
        KLOG_DEBUG(qLcIdleWatcher) << "idle watcher is already" << (enabled?"enabled":"disabled");
        return false;
    }
    
    KLOG_INFO(qLcIdleWatcher) << "set idle watcher enabled" << enabled;

    bool isActive = getIdleDetectionActive();
    if( !enabled && isActive )
    {
        setIdleDetectionActive(false);
    }

    m_enabled = enabled;
    return true;
}

void IdleWatcherMate::slotPresenceStatusChanged(uint status)
{
    auto iter  = PresenceStatusDescMap.find((PresenceStatus)status);
    KLOG_INFO(qLcIdleWatcher) << "recv session manager presence status changed: " << (iter==PresenceStatusDescMap.end()?"null":iter.value());
    setStatus(status);
}

void IdleWatcherMate::slotPresenceStatusTextChanged(const QString &statusText)
{
    KLOG_DEBUG(qLcIdleWatcher) << "presence status text changed: " << statusText;
    m_statusMsg = statusText;
}

void IdleWatcherMate::setStatus(uint status)
{
    bool isIdle = false;

    if (!m_idleDetectionActive)
    {
        KLOG_INFO(qLcIdleWatcher) << "not active, ignoring status changes";
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
        // 设置空闲预告，通知外部处理（抓取鼠标,屏幕变暗淡出）
        if( !setIdleNotice(true) )
        {
            return;
        }

        // 开启空闲延时定时器, 定时器过后将正式进入空闲状态
        m_idleTimerID = startTimer(m_delayIdleTimeout);
    }
    else
    {
        // 清理延时定时器
        if(m_idleTimerID!=0)
        {
            killTimer(m_idleTimerID);
            m_idleTimerID = 0;
        }

        // 复位状态
        setIdle(false);
        setIdleNotice(false);
    }
}

bool IdleWatcherMate::setIdleNotice(bool notice)
{
    bool  res = false;

    if (notice == m_idleNotice)
    {
        KLOG_DEBUG(qLcIdleWatcher) << "idle notice no changes,ignore it";
        return res;
    }

    KLOG_INFO(qLcIdleWatcher) << "idle notice changed: " << notice;
    emit IdleWatcher::idleNoticeChanged(notice,res);

    if(Q_LIKELY(res))
    {
        m_idleNotice = notice;
    }
    else
    {
        KLOG_INFO(qLcIdleWatcher) << "idle notice has been denied";
    }

    return res;
}

bool IdleWatcherMate::setIdle(bool idle)
{
    bool res = false;

    if(idle == m_idle)
    {
        KLOG_DEBUG(qLcIdleWatcher) << "idle no changes,ignore it";
        return res;
    }
    
    KLOG_INFO(qLcIdleWatcher) << "change idle state: " << idle;
    emit IdleWatcher::idleChanged(idle,res);

    if(Q_LIKELY(res))
    {
        KLOG_DEBUG(qLcIdleWatcher) << "change idle state: " << idle;
        m_idle = idle;
    }
    else
    {
        KLOG_DEBUG(qLcIdleWatcher) << "idle changed signal not handled:" << res;
    }

    return res;
}

void IdleWatcherMate::timerEvent(QTimerEvent *event)
{
    if(event->timerId() == m_idleTimerID)
    {
        slotHandleIdleDelayTimeout();
    }
    QObject::timerEvent(event);
}

void IdleWatcherMate::slotHandleIdleDelayTimeout()
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
