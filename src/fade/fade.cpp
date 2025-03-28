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
#include "fade.h"
#include "fade-gamma.h"
#include "fade-interface.h"
#include "fade-xrandr.h"

#include <qt5-log-i.h>
#include <QTimerEvent>

// 淡出时间 ms
#define FADE_TIMEOUT 2000
// 淡出动画的帧数
#define FADE_STEP_REP_SECOND 60

using namespace Kiran::ScreenSaver;

Fade::Fade(QObject *parent)
    : QObject(parent)
{
    if (FadeXrandr::checkForSupport())
    {
        m_fadeInterface = new FadeXrandr();
        KLOG_INFO() << "fade by xrandr...";
    }
    else if (FadeGamma::checkForSupport())
    {
        m_fadeInterface = new FadeGamma();
        KLOG_INFO() << "fade by gamma...";
    }
    else
    {
        KLOG_WARNING() << "fade out is not supported!";
    }
}

Fade::~Fade()
{
    delete m_fadeInterface;
}

bool Fade::getActive() const
{
    return m_active;
}

bool Fade::startAsync()
{
    KLOG_DEBUG() << "start fade out";

    if (!m_fadeInterface)
    {
        KLOG_ERROR() << "fade out isn't supported";
        return false;
    }

    if (m_active)
    {
        stop();
    }

    if (!m_fadeInterface->setup())
    {
        KLOG_ERROR() << "can't setup fade!";
        return false;
    }

    m_active = true;
    m_currentAlpha = 1.0;

    /// 总计多少步
    uint16_t num_steps = FADE_TIMEOUT / 1000.0 * FADE_STEP_REP_SECOND;

    /// 每一步需要多少毫秒
    uint16_t msecs_per_step = 1000 / FADE_STEP_REP_SECOND;

    /// 每一步改变的alpha值
    m_fadeStepAlpha = 1.0 / (qreal)num_steps;

    /// 开始定时器
    m_fadeTimerID = startTimer(msecs_per_step);

    return true;
}

void Fade::stop()
{
    KLOG_DEBUG() << "stopping fade";

    if (m_fadeTimerID > 0)
    {
        killTimer(m_fadeTimerID);
    }
    m_active = false;
    m_fadeTimerID = 0;
    m_fadeStepAlpha = 0.0;
}

void Fade::timerEvent(QTimerEvent *event)
{
    if (event->timerId() == m_fadeTimerID)
    {
        if (!handleFadeTimeout())
        {
            killTimer(m_fadeTimerID);
            m_fadeTimerID = 0;
            fadeFinish();
        }
    }
    QObject::timerEvent(event);
}

bool Fade::handleFadeTimeout()
{
    if (m_currentAlpha < 0.01)
    {
        KLOG_DEBUG() << "current alpha < 0.01,close timer";
        return false;
    }

    m_currentAlpha -= m_fadeStepAlpha;
    KLOG_DEBUG() << "handle fade timer timeout,set current alpha:" << m_currentAlpha;

    bool bres = m_fadeInterface->setAlphaGamma(m_currentAlpha);
    if (!bres)
    {
        KLOG_DEBUG() << "can't set alpha gamma,close timer";
    }
    return bres;
}

void Fade::fadeFinish()
{
    stop();
    KLOG_DEBUG() << "fade is finished";
    emit faded();
}

void Fade::reset()
{
    KLOG_DEBUG() << "reset fade";

    if (!m_fadeInterface)
    {
        KLOG_ERROR() << "fade out isn't supported";
        return;
    }

    // 停止淡出
    if (m_active)
    {
        stop();
    }

    // 未开始淡出，直接返回
    if( m_currentAlpha == 1.0 )
    {
        return;
    }

    // 复位gamma
    m_currentAlpha = 1.0;
    m_fadeInterface->setAlphaGamma(m_currentAlpha);

    // 释放
    m_fadeInterface->finish();
}
