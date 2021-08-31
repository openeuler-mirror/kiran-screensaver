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

#include "ks-screensaver.h"
#include "kiran-graphics-glow-effect.h"
#include "qt5-log-i.h"
#include "ui_ks-screensaver.h"

#include <QResizeEvent>
#include <QtMath>
#include <QStateMachine>
#include <QSignalTransition>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>

KSScreensaver::KSScreensaver(bool enableAnimation, QWidget *parent)
    : QWidget(parent),
      ui(new Ui::KSScreensaver),
      m_parentWidget(parent),
      m_enableAnimation(enableAnimation)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_TranslucentBackground);
    setMouseTracking(true);

    initGraphicsEffect();
    setupStateMachine();

    if(m_parentWidget!= nullptr)
    {
        m_parentWidget->installEventFilter(this);
        adjustGeometry(m_parentWidget->size());
    }
}

KSScreensaver::~KSScreensaver()
{
    delete ui;
}

void KSScreensaver::changeEvent(QEvent *event)
{
    if ((event->type() == QEvent::ParentChange) &&
        (parentWidget() != m_parentWidget))
    {
        if(m_parentWidget != nullptr)
        {
            m_parentWidget->removeEventFilter(this);
        }

        m_parentWidget = parentWidget();

        if (m_parentWidget != nullptr)
        {
            m_parentWidget->installEventFilter(this);
            adjustGeometry(m_parentWidget->size());
        }
    }
    QWidget::changeEvent(event);
}

void KSScreensaver::adjustGeometry(const QSize &size)
{
    KLOG_DEBUG() << "adjust geometry:" << size;
    QRect rect(0, m_isActive ? 0 : -size.height(), size.width(), size.height());
    setGeometry(rect);
    updateStateProperty();
}

void KSScreensaver::initGraphicsEffect()
{
    // 设置透明并且带有阴影(为了字体在白色背景下更清晰)
    m_opacityEffect = new KiranGraphicsGlowEffect(this);
    m_opacityEffect->setBlurRadius(8);
    m_opacityEffect->setDistance(0);
    this->setGraphicsEffect(m_opacityEffect);
}

void KSScreensaver::setupStateMachine()
{
    KLOG_DEBUG() << "setup state machine...";
    // 初始化状态机
    m_stateMachine = new QStateMachine(this);
    m_stateMachine->setAnimated(m_enableAnimation);
    m_activeState = new QState(m_stateMachine);
    m_unactiveState = new QState(m_stateMachine);

    // 激活状态属性设置
    m_activeState->assignProperty(m_opacityEffect,"opacity",QVariant(1.0));
    m_activeState->assignProperty(this,"geometry",QRect(0,0,this->width(),this->height()));
    auto toInactiveTransition = m_activeState->addTransition(this,SIGNAL(inactivation()),m_unactiveState);

    auto inActiveAnimationGroup = new QParallelAnimationGroup(m_stateMachine);
    toInactiveTransition->addAnimation(inActiveAnimationGroup);

    auto inActiveOpacityAnimation = new QPropertyAnimation(m_opacityEffect,"opacity");
    inActiveOpacityAnimation->setDuration(300);
    inActiveOpacityAnimation->setEasingCurve(QEasingCurve::InCubic);
    inActiveAnimationGroup->addAnimation(inActiveOpacityAnimation);

    auto inActiveGeometryAnimation = new QPropertyAnimation(this,"geometry");
    inActiveGeometryAnimation->setDuration(400);
    inActiveGeometryAnimation->setEasingCurve(QEasingCurve::InCubic);
    inActiveAnimationGroup->addAnimation(inActiveGeometryAnimation);

    // 非激活状态属性设置
    m_unactiveState->assignProperty(m_opacityEffect,"opacity",QVariant(0));
    m_unactiveState->assignProperty(this,"geometry",QRect(0,-height(),this->width(),this->height()));
    auto toActiveTransition = m_unactiveState->addTransition(this,SIGNAL(activation()),m_activeState);
    toActiveTransition->addAnimation(new QPropertyAnimation(m_opacityEffect,"opacity"));
    toActiveTransition->addAnimation(new QPropertyAnimation(this,"geometry"));

    // 初始化状态默认为激活
    m_stateMachine->setInitialState(m_activeState);
    // NOTE:状态机内部实现设置启动的话，会调用QMetaObject::invokeMethod,Qt::QueuedConnection,该任务会加入处理队列,状态可能不会即使生效
    m_stateMachine->setRunning(true);
}

bool KSScreensaver::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == m_parentWidget)
    {
        if(event->type() == QEvent::Resize)
        {
            //　父窗口更改大小，该窗口更新大小
            adjustGeometry(dynamic_cast<QResizeEvent*>(event)->size());
        }
    }
    return QObject::eventFilter(watched, event);
}

void KSScreensaver::updateStateProperty()
{
    KLOG_DEBUG() << "screensaver update state property:" << this->geometry();
    // 更新状态中对应的属性值
    QSize geoSize = this->geometry().size();
    m_unactiveState->assignProperty(this,"geometry",QRect(0,-geoSize.height(),geoSize.width(),geoSize.height()));
    m_activeState->assignProperty(this,"geometry",QRect(0,0,geoSize.width(),geoSize.height()));
}

bool KSScreensaver::active()
{
    return m_isActive;
}

void KSScreensaver::setActive(bool active)
{
    if(active == m_isActive)
    {
        return;
    }

    m_isActive = active;

    if(m_isActive)
    {
//        KLOG_DEBUG() << "inactive:"
        emit activation();
    }
    else
    {
        if(m_stateMachine->isRunning())
        {
            //　若状态机在运行，发送状态改变信号
            emit inactivation();
        }
        else
        {
            // 若状态机没在运行，则可直接更改初始化状态
            m_stateMachine->setInitialState(m_unactiveState);
        }
    }
}

void KSScreensaver::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
}