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
#include <QGraphicsOpacityEffect>
#include "qt5-log-i.h"
#include "ui_ks-screensaver.h"
#include "float-label.h"

#include <QDateTime>
#include <QParallelAnimationGroup>
#include <QPropertyAnimation>
#include <QResizeEvent>
#include <QSignalTransition>
#include <QStateMachine>
#include <QTimer>
#include <QtMath>

KSScreensaver::KSScreensaver(bool enableAnimation, QWidget *parent)
    : QWidget(parent),
      ui(new Ui::KSScreensaver),
      m_enableAnimation(enableAnimation)
{
    ui->setupUi(this);
    init();
}

KSScreensaver::~KSScreensaver()
{
    delete ui;
}

bool KSScreensaver::maskState()
{
    return m_masked;
}

void KSScreensaver::setMaskState(bool maskState)
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
        m_floatingLabel->start();
    }
    else
    {
        m_floatingLabel->reset();
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

void KSScreensaver::init()
{
    // 背景透明
    setAttribute(Qt::WA_TranslucentBackground);
    setMouseTracking(true);

    //　屏保悬浮提示
    m_floatingLabel = new FloatLabel(this);
    m_floatingLabel->setAnchor(0.5,0.93);
    m_floatingLabel->setFloatingParameter(FloatLabel::DIRECTION_UP,true,2600,24);
    m_floatingLabel->setText("鼠标点击即可解锁");
    m_floatingLabel->setPixmap(QPixmap(":/kiran-screensaver/images/arrow.svg"),QSize(16,16));
    m_floatingLabel->start();

    // 初始化图形效果
    initGraphicsEffect();

    // 安装状态机
    setupStateMachine();

    // 开始更新时间
    startUpdateTimeDateTimer();
}

void KSScreensaver::initGraphicsEffect()
{
    // 设置透明并且带有阴影(为了字体在白色背景下更清晰)
    m_opacityEffect = new KiranGraphicsGlowEffect(this);
    m_opacityEffect->setXOffset(2);
    m_opacityEffect->setYOffset(2);
    m_opacityEffect->setBlurRadius(2);
    this->setGraphicsEffect(m_opacityEffect);
}

void KSScreensaver::setupStateMachine()
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
    inActiveOpacityAnimation->setDuration(300);
    inActiveOpacityAnimation->setEasingCurve(QEasingCurve::InCubic);
    inActiveAnimationGroup->addAnimation(inActiveOpacityAnimation);

    auto inActiveGeometryAnimation = new QPropertyAnimation(this, "geometry");
    inActiveGeometryAnimation->setDuration(400);
    inActiveGeometryAnimation->setEasingCurve(QEasingCurve::InCubic);
    inActiveAnimationGroup->addAnimation(inActiveGeometryAnimation);

    // 非激活状态属性设置
    m_unMaskState->assignProperty(m_opacityEffect, "opacity", QVariant(0));
    m_unMaskState->assignProperty(this, "geometry", QRect(0, -height(), this->width(), this->height()));
    auto toActiveTransition = m_unMaskState->addTransition(this, SIGNAL(masking()), m_maskState);
    toActiveTransition->addAnimation(new QPropertyAnimation(m_opacityEffect, "opacity"));
    toActiveTransition->addAnimation(new QPropertyAnimation(this, "geometry"));

    // 初始化状态默认为激活
    m_stateMachine->setInitialState(m_maskState);
    // NOTE:状态机内部实现设置启动的话，会调用QMetaObject::invokeMethod,Qt::QueuedConnection,该任务会加入处理队列,状态可能不会即使生效
    m_stateMachine->setRunning(true);
}

void KSScreensaver::updateStateProperty()
{
    KLOG_DEBUG() << "screensaver update state property:" << this->geometry();
    // 更新状态中对应的属性值
    QSize geoSize = this->geometry().size();
    m_unMaskState->assignProperty(this, "geometry", QRect(0, -geoSize.height(), geoSize.width(), geoSize.height()));
    m_maskState->assignProperty(this, "geometry", QRect(0, 0, geoSize.width(), geoSize.height()));
}

void KSScreensaver::startUpdateTimeDateTimer()
{
    QDateTime dateTime = QDateTime::currentDateTime();
    QString time = dateTime.toString("hh:mm");
    QString date = dateTime.toString(tr("MM-dd dddd"));
    ui->label_time->setText(time);
    ui->label_date->setText(date);

    QTime curTime = dateTime.time();
    int nextUpdateSecond = 60 - curTime.second();
    QTimer::singleShot(nextUpdateSecond*1000,this,&KSScreensaver::startUpdateTimeDateTimer);
}

void KSScreensaver::resizeEvent(QResizeEvent *event)
{
    dynamic_cast<QResizeEvent*>(event)->size();
    updateStateProperty();
    QWidget::resizeEvent(event);
}