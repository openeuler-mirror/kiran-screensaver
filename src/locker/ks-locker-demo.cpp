//
// Created by lxh on 2021/8/30.
//

#include "ks-locker-demo.h"
#include <QEvent>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>
#include <QResizeEvent>
#include <qt5-log-i.h>

#include "ui_ks-locker-demo.h"

KSLockerDemo::KSLockerDemo(bool enableAnimation, QWidget *parent)
    : QWidget(parent),
      ui(new Ui::KSLockerDemo),
      m_parentWidget(parent),
      m_enableAnimation(enableAnimation)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_TranslucentBackground);

    if (m_parentWidget != nullptr)
    {
        m_parentWidget->installEventFilter(this);
        adjustGeometry(m_parentWidget->size());
    }
    init();
}

KSLockerDemo::~KSLockerDemo()
{
    delete ui;
}

bool KSLockerDemo::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == m_parentWidget &&
        event->type() == QEvent::Resize)
    {
        auto ev = dynamic_cast<QResizeEvent*>(event);
        adjustGeometry(ev->size());
    }
    return QObject::eventFilter(watched, event);
}

void KSLockerDemo::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
}

void KSLockerDemo::changeEvent(QEvent *event)
{
    if ((event->type() == QEvent::ParentChange) &&
        (parentWidget() != m_parentWidget))
    {
        if (m_parentWidget != nullptr)
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

bool KSLockerDemo::active() const
{
    return m_active;
}

void KSLockerDemo::setActive(bool active)
{
    if(m_active == active)
    {
        return;
    }

    KLOG_DEBUG() << "set active:" << active;
    m_active = active;

    if(m_enableAnimation)
    {
        m_animation->setDuration(m_active?600:50);
        m_animation->setDirection(m_active?QAbstractAnimation::Forward:QAbstractAnimation::Backward);
        m_animation->start();
    }
    else
    {
        m_opacityEffect->setOpacity(m_active?1:0);
    }
}

void KSLockerDemo::init()
{
    //　设置透明效果
    m_opacityEffect = new QGraphicsOpacityEffect(this);
    m_opacityEffect->setOpacity(0);
    setGraphicsEffect(m_opacityEffect);

    // 渐出动画
    if( m_enableAnimation )
    {
        m_animation = new QPropertyAnimation(this);
        m_animation->setTargetObject(m_opacityEffect);
        m_animation->setPropertyName("opacity");
        m_animation->setStartValue(0);
        m_animation->setEndValue(1);
    }
}

void KSLockerDemo::adjustGeometry(const QSize &size)
{
    QRect rect(0,  0 , size.width(), size.height());
    setGeometry(rect);
}
