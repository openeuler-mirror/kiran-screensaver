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

#include "screensaver.h"
#include <QGraphicsOpacityEffect>
#include "animation-define.h"
#include "float-label.h"
#include "graphics-glow-effect.h"
#include "qt5-log-i.h"
#include "ui_screensaver.h"

#include <QDateTime>
#include <QParallelAnimationGroup>
#include <QPropertyAnimation>
#include <QResizeEvent>
#include <QSignalTransition>
#include <QStateMachine>
#include <QTimer>
#include <QtMath>

using namespace Kiran::ScreenSaver;

Screensaver::Screensaver(bool enableAnimation, QWidget *parent)
    : QWidget(parent),
      ui(new Ui::Screensaver),
      m_enableAnimation(enableAnimation)
{
    ui->setupUi(this);
    init();
}

Screensaver::~Screensaver()
{
    delete ui;
}

bool Screensaver::maskState()
{
    return m_masked;
}

void Screensaver::setMaskState(bool maskState)
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
//        m_floatingLabel->start();
    }
    else
    {
//        m_floatingLabel->reset();
        if (m_stateMachine->isRunning())
        {
            //　若状态机在运行，发送状态改变信号
            emit unmasking();
        }
        else
        {
            // 若状态机没在运行，则可直接更改初始化状态
            m_stateMachine->setInitialState(m_unMaskState);
        }
    }
}

void Screensaver::init()
{
    // 背景透明
    setAttribute(Qt::WA_TranslucentBackground);
    setMouseTracking(true);

    //　屏保悬浮提示
    m_floatingLabel = new FloatLabel(this);
    m_floatingLabel->setAnchor(0.5,0.93);
    m_floatingLabel->setFloatingParameter(FloatLabel::DIRECTION_UP,true,2600,24);
    m_floatingLabel->setText(tr("Click to unlock"));
    m_floatingLabel->setPixmap(QPixmap(":/kiran-screensaver/images/arrow.svg"),QSize(16,16));
//    m_floatingLabel->start();

    initGraphicsEffect();
    setupStateMachine();
    startUpdateTimeDateTimer();
}

void Screensaver::initGraphicsEffect()
{
    // 设置透明并且带有阴影(为了字体在白色背景下更清晰)
    m_opacityEffect = new GraphicsGlowEffect(this);
    m_opacityEffect->setXOffset(2);
    m_opacityEffect->setYOffset(2);

    // NOTE: 
    // 由于该Effect会占用过多cpu资源
    // 禁用GraphicsGlowEffect，在GraphicsGlowEffect::draw中会直接走drawSource
    m_opacityEffect->setDistance(0);
    m_opacityEffect->setBlurRadius(0);

    this->setGraphicsEffect(m_opacityEffect);
}

void Screensaver::setupStateMachine()
{
    KLOG_DEBUG() << "setup state machine...";

    // 初始化状态机
    m_stateMachine = new QStateMachine(this);
    m_stateMachine->setAnimated(m_enableAnimation);
    m_maskState = new QState(m_stateMachine);
    m_unMaskState = new QState(m_stateMachine);

    // 激活状态属性设置
    m_maskState->assignProperty(m_opacityEffect, "opacity", QVariant(1.0));
    m_maskState->assignProperty(this, "geometry", QRect(0, 0, this->width(), this->height()));
    auto toInactiveTransition = m_maskState->addTransition(this, SIGNAL(unmasking()), m_unMaskState);

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

    // 非激活状态属性设置
    m_unMaskState->assignProperty(m_opacityEffect, "opacity", QVariant(0));
    m_unMaskState->assignProperty(this, "geometry", QRect(0, -height(), this->width(), this->height()));

    auto toActiveTransition = m_unMaskState->addTransition(this, SIGNAL(masking()), m_maskState);

    auto toActiveOpacityAnimation = new QPropertyAnimation(m_opacityEffect,"opacity");
    toActiveOpacityAnimation->setDuration(SCREENSAVER_DOWN_SLIP_ANIMATION_DURATION_MS);
    toActiveOpacityAnimation->setEasingCurve(QEasingCurve::OutCubic);
    toActiveTransition->addAnimation(toActiveOpacityAnimation);

    auto toActiveGeometryAnimation = new QPropertyAnimation(this,"geometry");
    toActiveGeometryAnimation->setDuration(SCREENSAVER_DOWN_SLIP_ANIMATION_DURATION_MS);
    toActiveGeometryAnimation->setEasingCurve(QEasingCurve::OutCubic);
    toActiveTransition->addAnimation(toActiveGeometryAnimation);

    // 初始化状态默认为激活
    m_stateMachine->setInitialState(m_maskState);
    // NOTE:状态机内部实现设置启动的话，会调用QMetaObject::invokeMethod,Qt::QueuedConnection,该任务会加入处理队列,状态可能不会即使生效
    m_stateMachine->setRunning(true);
}

void Screensaver::updateStateProperty()
{
    KLOG_DEBUG() << "screensaver update state property:" << this->geometry();
    // 更新状态中对应的属性值
    QSize geoSize = this->geometry().size();
    m_unMaskState->assignProperty(this, "geometry", QRect(0, -geoSize.height(), geoSize.width(), geoSize.height()));
    m_maskState->assignProperty(this, "geometry", QRect(0, 0, geoSize.width(), geoSize.height()));
}

void Screensaver::startUpdateTimeDateTimer()
{
    QDateTime dateTime = QDateTime::currentDateTime();
    QString time = dateTime.toString("hh:mm");
    QString date = dateTime.toString(tr("MM-dd dddd"));
    ui->label_time->setText(time);
    ui->label_date->setText(date);

    QTime curTime = dateTime.time();
    int nextUpdateSecond = 60 - curTime.second();
    QTimer::singleShot(nextUpdateSecond*1000,this,&Screensaver::startUpdateTimeDateTimer);
}

void Screensaver::resizeEvent(QResizeEvent *event)
{
    dynamic_cast<QResizeEvent*>(event)->size();
    updateStateProperty();
    QWidget::resizeEvent(event);
}