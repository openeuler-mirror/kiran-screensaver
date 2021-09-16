//
// Created by lxh on 2021/7/2.
//

#include "ks-fade.h"
#include "ks-fade-xrandr.h"
#include "ks-fade-gamma.h"

#include <qt5-log-i.h>
#include <QTimerEvent>

// 淡出时间 ms
#define FADE_TIMEOUT 2000

// 淡出动画的帧数
#define FADE_STEP_REP_SECOND 60

KSFade::KSFade(QObject *parent)
    :QObject(parent)
{
    if(KSFadeXrandr::checkForSupport())
    {
        m_fadeInterface = new KSFadeXrandr();
        KLOG_DEBUG() << "fade by xrandr...";
    }
    else if(KSFadeGamma::checkForSupport())
    {
        m_fadeInterface = new KSFadeGamma();
        KLOG_DEBUG() << "fade by gamma...";
    }
    else
    {
        KLOG_WARNING() << "fade out is not supported!";
    }
}

KSFade::~KSFade()
{
    delete m_fadeInterface;
    m_fadeInterface = nullptr;
}

bool KSFade::getActive() const
{
    return m_active;
}

bool KSFade::startAsync()
{
    KLOG_DEBUG() << "start fade out";

    if(!m_fadeInterface)
    {
        KLOG_WARNING() << "fade out isn't supported";
        return false;
    }

    if( m_active )
    {
        stop();
    }

    if( !m_fadeInterface->setup() )
    {
        KLOG_ERROR() << "can't setup fade!";
        return false;
    }

    m_active = true;
    m_currentAlpha = 1.0;

    ///总计多少步
    uint16_t num_steps = FADE_TIMEOUT/1000.0 * FADE_STEP_REP_SECOND;

    ///每一步需要多少毫秒
    uint16_t msecs_per_step = 1000/FADE_STEP_REP_SECOND;

    ///每一步改变的alpha值
    m_fadeStepAlpha = 1.0/(qreal)num_steps;

    /// 开始定时器
    m_fadeTimerID = startTimer(msecs_per_step);

    return true;
}

void KSFade::stop()
{
    KLOG_DEBUG() << "stopping fade";

    if( m_fadeTimerID>0 )
    {
        killTimer(m_fadeTimerID);
    }
    m_active = false;
    m_fadeTimerID = 0;
    m_fadeStepAlpha = 0.0;
}

void KSFade::timerEvent(QTimerEvent *event)
{
    if( event->timerId() == m_fadeTimerID )
    {
        if( !handleFadeTimeout() )
        {
            killTimer(m_fadeTimerID);
            m_fadeTimerID = 0;
            fadeFinish();
        }
    }
    QObject::timerEvent(event);
}

bool KSFade::handleFadeTimeout()
{
    if( m_currentAlpha < 0.01 )
    {
        KLOG_DEBUG() << "current alpha < 0.01,close timer";
        return false;
    }

    m_currentAlpha -= m_fadeStepAlpha;
    KLOG_DEBUG() << "handle fade timer timeout,set current alpha:" << m_currentAlpha;

    bool bres = m_fadeInterface->setAlphaGamma(m_currentAlpha);
    if(!bres)
    {
        KLOG_DEBUG() << "can't set alpha gamma,close timer";
    }
    return bres;
}

void KSFade::fadeFinish()
{
    stop();
    KLOG_DEBUG() << "fade is finished";
    emit faded();
}

void KSFade::reset()
{
    KLOG_DEBUG() << "reset fade";

    //停止淡出
    if(m_active)
    {
        stop();
    }

    //复位gamma
    m_currentAlpha = 1.0;
    m_fadeInterface->setAlphaGamma(m_currentAlpha);

    //释放
    m_fadeInterface->finish();
}
