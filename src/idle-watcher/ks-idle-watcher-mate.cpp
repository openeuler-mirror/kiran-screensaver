//
// Created by lxh on 2021/6/28.
//

#include "ks-idle-watcher-mate.h"
#include <qt5-log-i.h>

using namespace org::gnome::SessionManager;

///该枚举定义在mate-session-manager-1.22.1 gsm-presence.h中，由于未提供头文件，故将其拷贝定义在此处
enum PresenceStatus
{
    PRESENCE_STATUS_AVAILABLE = 0,
    PRESENCE_STATUS_INVISIBLE,
    PRESENCE_STATUS_BUSY,
    PRESENCE_STATUS_IDLE
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
    QDBusConnection sessionBus = QDBusConnection::sessionBus();
    if (!sessionBus.isConnected())
    {
        KLOG_ERROR() << "can't connect to session bus!";
        return false;
    }

    delete m_presenceInterface;
    m_presenceInterface = nullptr;
    m_presenceInterface = new Presence("org.gnome.SessionManager", "/org/gnome/SessionManager/Presence", sessionBus, this);
    if (!connect(m_presenceInterface, &Presence::StatusChanged, this, &KSIdleWatcherMate::slotPresenceStatusChanged))
    {
        KLOG_ERROR() << "can't connect status changed signal!";
        return false;
    }
    if (!connect(m_presenceInterface, &Presence::StatusTextChanged, this, &KSIdleWatcherMate::slotPresenceStatusTextChanged))
    {
        KLOG_ERROR() << "can't connect status text changed signal!";
        return false;
    }

    return true;
}

bool KSIdleWatcherMate::getActive()
{
    return m_active;
}

bool KSIdleWatcherMate::setActive(bool active)
{
    KLOG_DEBUG() << "turning idle watcher: " << (active?"ON":"OFF");

    if( active == m_active )
    {
        KLOG_DEBUG() << "idle watcher is already" << (active?"active":"inactive");
        return false;
    }

    if( !m_enabled )
    {
        KLOG_DEBUG() << "idle watcher is disabled,cannot active";
        return false;
    }

    m_idle = false;
    m_idleNotice = false;
    if(m_idleTimerID != 0)
    {
        killTimer(m_idleTimerID);
    }
    m_active = active;
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

    bool isActive = getActive();
    if( !enabled && isActive )
    {
        setActive(false);
    }
    m_enabled = enabled;

    return true;
}

void KSIdleWatcherMate::slotPresenceStatusChanged(uint status)
{
    KLOG_DEBUG() << "presence status changed: " << status;
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

    if (!m_active)
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
        killTimer(m_idleTimerID);
        m_idleTimerID = 0;
    }

    if (isIdle)
    {
        setIdleNotice(true);
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
