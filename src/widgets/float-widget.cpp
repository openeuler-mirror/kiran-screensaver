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

#include "float-widget.h"
#include <QEvent>
#include <QStyleOption>
#include <QPainter>
#include <QResizeEvent>
#include <QPropertyAnimation>
#include <QSequentialAnimationGroup>
#include <QParallelAnimationGroup>
#include <QApplication>
using namespace Kiran::ScreenSaver;
FloatWidget::FloatWidget(QWidget *parent)
    :QWidget(parent)
{
    m_opacityEffect = new QGraphicsOpacityEffect(this);
    setGraphicsEffect(m_opacityEffect);

    initAnimation();
    qApp->installEventFilter(this);
}

FloatWidget::~FloatWidget()
{

}

void FloatWidget::setAnchor(qreal xAxisPercentage, qreal yAxisPercentage)
{
    m_xAxisPercent = xAxisPercentage;
    m_yAxisPercent = yAxisPercentage;
    updateCurrentPosition();
}

void FloatWidget::setFloatingParameter(FloatWidget::FloatingDirection direction, bool floatingOpacity,
                                       int durationMs,
                                       int distancePx)
{
    m_floatingDirection = direction;
    m_durationMs = durationMs;
    m_distancePx = distancePx;
    m_enableFloatingOpacity = floatingOpacity;

    initAnimation();
}

void FloatWidget::start()
{
    if(m_floatingAnimation!=nullptr
       && m_floatingAnimation->state()!=QAbstractAnimation::Running)
    {
        m_floatingAnimation->start();
    }
}

void FloatWidget::stop()
{
    if(m_floatingAnimation!=nullptr
            && m_floatingAnimation->state()!=QAbstractAnimation::Paused)
    {
        m_floatingAnimation->stop();
    }
}

void FloatWidget::reset()
{
    if(m_floatingAnimation!=nullptr
            && m_floatingAnimation->state()==QAbstractAnimation::Running)
    {
        m_floatingAnimation->stop();
    }
    m_floatingDistance = 0;
    updateCurrentPosition();
}

int FloatWidget::floatingDistance() const
{
    return m_floatingDistance;
}

bool FloatWidget::eventFilter(QObject *watched, QEvent *event)
{
    if(parentWidget()!=nullptr&&watched==parentWidget()&&event->type()==QEvent::Resize)
    {
        updateCurrentPosition();
    }
    return QWidget::eventFilter(watched,event);
}

void FloatWidget::initAnimation()
{
    if(m_floatingAnimation!=nullptr)
    {
        m_floatingAnimation->stop();
        delete m_floatingAnimation;
        m_floatingAnimation = nullptr;
    }

    QSequentialAnimationGroup* animationGroup = new QSequentialAnimationGroup(this);
    animationGroup->setLoopCount(-1);

    auto upperAnimationGroup = new QParallelAnimationGroup(animationGroup);
    {
        auto upperAnimation = new QPropertyAnimation(upperAnimationGroup);
        upperAnimation->setTargetObject(this);
        upperAnimation->setPropertyName("floatingDistance");
        upperAnimation->setDuration(m_durationMs*(3/5.0));
        upperAnimation->setStartValue(0);
        upperAnimation->setEndValue(-m_distancePx);
        upperAnimation->setEasingCurve(QEasingCurve::OutQuad);
        upperAnimationGroup->addAnimation(upperAnimation);

        if(m_enableFloatingOpacity)
        {
            auto upperOpactiyAnimation = new QPropertyAnimation(upperAnimationGroup);
            upperOpactiyAnimation->setTargetObject(m_opacityEffect);
            upperOpactiyAnimation->setPropertyName("opacity");
            upperOpactiyAnimation->setDuration(m_durationMs*(3/5.0));
            upperOpactiyAnimation->setStartValue(0.2);
            upperOpactiyAnimation->setEndValue(0.6);
            upperOpactiyAnimation->setEasingCurve(QEasingCurve::OutQuad);
            upperAnimationGroup->addAnimation(upperOpactiyAnimation);
        }
    }
    animationGroup->addAnimation(upperAnimationGroup);

    auto downAnimationGroup = new QParallelAnimationGroup(animationGroup);
    {
        auto downAnimation = new QPropertyAnimation(downAnimationGroup);
        downAnimation->setTargetObject(this);
        downAnimation->setPropertyName("floatingDistance");
        downAnimation->setDuration((m_durationMs/5)*2);
        downAnimation->setStartValue(-m_distancePx);
        downAnimation->setEndValue(0);
        downAnimation->setEasingCurve(QEasingCurve::InQuad);
        downAnimationGroup->addAnimation(downAnimation);

        if(m_enableFloatingOpacity)
        {
            auto downOpacityAnimation = new QPropertyAnimation(downAnimationGroup);
            downOpacityAnimation->setTargetObject(m_opacityEffect);
            downOpacityAnimation->setPropertyName("opacity");
            downOpacityAnimation->setDuration((m_durationMs/5)*2);
            downOpacityAnimation->setStartValue(0.6);
            downOpacityAnimation->setEndValue(0.2);
            downOpacityAnimation->setEasingCurve(QEasingCurve::InQuad);
            downAnimationGroup->addAnimation(downOpacityAnimation);
        }
    }
    animationGroup->addAnimation(downAnimationGroup);

    m_floatingAnimation = animationGroup;
}

void FloatWidget::updateCurrentPosition()
{
    if(parentWidget() == nullptr)
    {
        return;
    }

    QRect parentRect = parentWidget()->rect();

    QPoint newCenter( (parentRect.width()*m_xAxisPercent),
                     (parentRect.height()*m_yAxisPercent)+m_floatingDistance);

    QRect newRect = this->rect();
    newRect.moveCenter(newCenter);

    this->setGeometry(newRect);
}

void FloatWidget::setFloatingDistance(int floatingDistance)
{
    m_floatingDistance = floatingDistance;
    updateCurrentPosition();
}

void FloatWidget::changeEvent(QEvent *event)
{
    if( event->type() == QEvent::ParentChange )
    {
        updateCurrentPosition();
    }
}

void FloatWidget::paintEvent(QPaintEvent *event)
{
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}


