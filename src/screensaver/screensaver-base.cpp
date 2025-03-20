/**
 * Copyright (c) 2020 ~ 2025 KylinSec Co., Ltd.
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
#include "screensaver-base.h"
#include <QDateTime>
#include <QParallelAnimationGroup>
#include <QPropertyAnimation>
#include <QResizeEvent>
#include <QSignalTransition>
#include <QStateMachine>
#include <QTimer>
#include "animation-define.h"
#include "graphics-glow-effect.h"
#include "qt5-log-i.h"
#include <QPainter>

namespace Kiran
{
namespace ScreenSaver
{
ScreensaverBase::ScreensaverBase(bool animated, QWidget *parent)
    : QWidget(parent),
      m_enableAnimation(animated)
{
    init();
}

ScreensaverBase::~ScreensaverBase() = default;

bool ScreensaverBase::isMasked()
{
    return m_masked;
}

void ScreensaverBase::setMaskState(bool maskState)
{
    if (maskState == m_masked)
    {
        return;
    }

    KLOG_DEBUG() << "set mask state:" << (maskState ? "mask" : "unmask");
    m_masked = maskState;

    if (m_masked)
    {
        emit masking();
    }
    else
    {
        if (m_stateMachine->isRunning())
        {
            // 　若状态机在运行，发送状态改变信号
            emit unmasking();
        }
        else
        {
            // 若状态机没在运行，则可直接更改初始化状态
            m_stateMachine->setInitialState(m_unmaskState);
        }
    }
}

void ScreensaverBase::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    KLOG_INFO() << "resize event:" << dynamic_cast<QResizeEvent *>(event)->size();
    updateStateProperty();
}

void ScreensaverBase::init()
{
    // 背景透明
    setAttribute(Qt::WA_TranslucentBackground);

    // 鼠标追踪显示
    setMouseTracking(true);

    // NOTE:
    // 由于该Effect会占用过多cpu资源
    // 禁用distance+blurRadius不走阴影生成，在GraphicsGlowEffect::draw中会直接走drawSource
    m_opacityEffect = new GraphicsGlowEffect(this);
    m_opacityEffect->setXOffset(2);
    m_opacityEffect->setYOffset(2);
    m_opacityEffect->setDistance(0);
    m_opacityEffect->setBlurRadius(0);
    this->setGraphicsEffect(m_opacityEffect);

    initStateMachine();
}

void ScreensaverBase::initStateMachine()
{
    // 设置StateMachine
    m_stateMachine = new QStateMachine(this);
    m_stateMachine->setAnimated(m_enableAnimation);
    m_maskState = new QState(m_stateMachine);
    m_unmaskState = new QState(m_stateMachine);

    // mask state
    m_maskState->assignProperty(m_opacityEffect, "opacity", QVariant(1.0));
    m_maskState->assignProperty(this, "geometry", QRect(0, 0, this->width(), this->height()));
    auto toInactiveTransition = m_maskState->addTransition(this, &ScreensaverBase::unmasking, m_unmaskState);

    auto inActiveAnimationGroup = new QParallelAnimationGroup(m_stateMachine);
    toInactiveTransition->addAnimation(inActiveAnimationGroup);

    auto inActiveOpacityAnimation = new QPropertyAnimation(m_opacityEffect, "opacity");
    inActiveOpacityAnimation->setDuration(SCREENSAVER_UP_SLIP_ANIMATION_DURATION_MS);
    inActiveOpacityAnimation->setEasingCurve(QEasingCurve::InCubic);
    inActiveAnimationGroup->addAnimation(inActiveOpacityAnimation);

    auto inActiveGeometryAnimation = new QPropertyAnimation(this, "geometry");
    inActiveGeometryAnimation->setDuration(SCREENSAVER_UP_SLIP_ANIMATION_DURATION_MS);
    inActiveGeometryAnimation->setEasingCurve(QEasingCurve::InCubic);
    inActiveAnimationGroup->addAnimation(inActiveGeometryAnimation);

    // unmask state
    m_unmaskState->assignProperty(m_opacityEffect, "opacity", QVariant(0));
    m_unmaskState->assignProperty(this, "geometry", QRect(0, -height(), this->width(), this->height()));

    auto toActiveTransition = m_unmaskState->addTransition(this, &ScreensaverBase::masking, m_maskState);

    auto toActiveOpacityAnimation = new QPropertyAnimation(m_opacityEffect, "opacity");
    toActiveOpacityAnimation->setDuration(SCREENSAVER_DOWN_SLIP_ANIMATION_DURATION_MS);
    toActiveOpacityAnimation->setEasingCurve(QEasingCurve::OutCubic);
    toActiveTransition->addAnimation(toActiveOpacityAnimation);

    auto toActiveGeometryAnimation = new QPropertyAnimation(this, "geometry");
    toActiveGeometryAnimation->setDuration(SCREENSAVER_DOWN_SLIP_ANIMATION_DURATION_MS);
    toActiveGeometryAnimation->setEasingCurve(QEasingCurve::OutCubic);
    toActiveTransition->addAnimation(toActiveGeometryAnimation);

    // 初始化状态默认为激活
    m_stateMachine->setInitialState(m_maskState);
    // NOTE:状态机内部实现设置启动的话，会调用QMetaObject::invokeMethod,Qt::QueuedConnection,该任务会加入处理队列,状态可能不会即使生效
    m_stateMachine->setRunning(true);
}

void ScreensaverBase::updateStateProperty()
{
    KLOG_INFO() << "screensaver update state property:" << this->geometry();
    // 更新状态中对应的属性值
    QSize geoSize = this->geometry().size();
    m_unmaskState->assignProperty(this, "geometry", QRect(0, -geoSize.height(), geoSize.width(), geoSize.height()));
    m_maskState->assignProperty(this, "geometry", QRect(0, 0, geoSize.width(), geoSize.height()));
}

void ScreensaverBase::startUpdateTimeDateTimer()
{
    QDateTime dateTime = QDateTime::currentDateTime();
    QTime curTime = dateTime.time();

    int nextUpdateSecond = 60 - curTime.second();
    emit updateTime();

    QTimer::singleShot(nextUpdateSecond * 1000, this, &ScreensaverBase::startUpdateTimeDateTimer);
}

}  // namespace ScreenSaver
}  // namespace Kiran