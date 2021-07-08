//
// Created by lxh on 2021/6/25.
//

#include "ks-listener.h"
#include "logind-session-monitor.h"

#include <qt5-log-i.h>

#define NOT_SUPPORTED_METHOD \
    {                        \
        KLOG_DEBUG("isn't supported!,ignore method call"); \
        sendErrorReply(QDBusError::NotSupported,"not supported!"); \
    }

KSListener::KSListener(QObject *parent) : QObject(parent)
{
}

KSListener::~KSListener()
{
}

bool KSListener::init()
{
    m_sessionMonitor = new LogindSessionMonitor(this);

    connect(m_sessionMonitor,&LogindSessionMonitor::Lock,
            this,&KSListener::handleLogindSessionLock);
    connect(m_sessionMonitor,&LogindSessionMonitor::Unlock,
            this, &KSListener::handleLogindSessionUnlock);

//    return m_sessionMonitor->init();
    return true;
}

bool KSListener::isInhibited()
{
    ///判断是否存在抑制器
    return false;
}

bool KSListener::setSessionIdle(bool idle)
{
    bool res;

    KLOG_DEBUG("set session %s",(idle?"idle":"not idle"));

    ///相同状态忽略
    if( m_sessionIdle == idle )
    {
        KLOG_DEBUG("trying to set idle state when already %s",(idle?"idle":"not idle"));
        return false;
    }

    if( idle )
    {
        if( isInhibited() )
        {
            ///由非空闲->空闲被抑制，设置会话空闲失败
            KLOG_DEBUG("try to set session idle failed,inhibited!");
            return false;
        }
    }

    m_sessionIdle = idle;
    res = setActiveStatus(m_sessionIdle);
    if( res )
    {
        m_sessionIdleStart = m_sessionIdle?time(nullptr):0;
    }
    else
    {
        m_sessionIdle = !idle;
    }

    return res;
}

void KSListener::Cycle()
{
    NOT_SUPPORTED_METHOD
}

uint KSListener::Throttle(const QString &application_name, const QString &reason)
{
    NOT_SUPPORTED_METHOD
    return 0;
}

void KSListener::UnThrottle(uint cookie)
{
    NOT_SUPPORTED_METHOD
}

void KSListener::ShowMessage(const QString &summary, const QString &body, const QString &icon)
{
    NOT_SUPPORTED_METHOD
}

void KSListener::SetActive(bool value)
{
    setActiveStatus(value);
}

bool KSListener::GetActive()
{
    return m_isActive;
}

uint KSListener::GetActiveTime()
{
    //获取当前时间戳-激活时间的时间戳
    return 0;
}

QStringList KSListener::GetInhibitors()
{
    return QStringList();
}

uint KSListener::Inhibit(const QString &application_name, const QString &reason)
{
    return 0;
}

void KSListener::Lock()
{
    emit sigLock();
}

void KSListener::Unlock()
{
    setActiveStatus(false);
}

void KSListener::SimulateUserActivity()
{

}

void KSListener::UnInhibit(uint cookie)
{

}

uint KSListener::generateCookie()
{
    time_t randomSeed = time(nullptr);
    qsrand(randomSeed);
    uint cookie =  (uint)qrand();
    return cookie;
}


void KSListener::handleLogindSessionLock()
{
}

void KSListener::handleLogindSessionUnlock()
{
}

bool KSListener::setActiveStatus(bool active)
{
    if(m_isActive == active)
    {
        KLOG_DEBUG("trying to set active state when already: %s",active ? "active" : "inactive");
        return false;
    }

    /// 发送信号，KSManager接收该信号(Qt::DirectConnection连接方式),
    /// 通过判断是否允许激活屏保或激活屏保成功
    /// 若激活失败，不对KSListener Active状态进行修改
    bool handled = false;
    emit sigActiveChanged(active,handled);
    if( !handled )
    {
        KLOG_DEBUG("active changed signal not handled,update active(%s) failed!",active ? "true" : "false");
        return false;
    }

    m_isActive = active;
    if(m_sessionIdle!=m_isActive)
    {
        m_sessionIdle = m_isActive;
    }

    m_activeStart = m_isActive?time(nullptr):0;
    emit ActiveChanged(m_isActive);
    return true;
}
