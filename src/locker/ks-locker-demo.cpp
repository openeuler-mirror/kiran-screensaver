//
// Created by lxh on 2021/8/30.
//

#include "ks-locker-demo.h"
#include <qt5-log-i.h>
#include <QEvent>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>
#include <QResizeEvent>

#include "ui_ks-locker-demo.h"

KSLockerDemo::KSLockerDemo(bool enableAnimation, QWidget *parent)
    : QWidget(parent),
      ui(new Ui::KSLockerDemo),
      m_parentWidget(parent),
      m_enableAnimation(enableAnimation)
{
    ui->setupUi(this);
    qApp->installEventFilter(this);
    init();
}

KSLockerDemo::~KSLockerDemo()
{
    delete ui;
}

bool KSLockerDemo::eventFilter(QObject *watched, QEvent *event)
{
    if(watched!= nullptr &&
        watched->isWidgetType() )
    {
        //　父窗口更改大小
        if(watched == m_parentWidget && event->type()==QEvent::Resize)
        {
            auto ev = dynamic_cast<QResizeEvent *>(event);
            adjustGeometry(ev->size());
        }

        // 窗口在显示过程中，还未完全显示时屏蔽所有输入事件
        QWidget* watchedWidget = qobject_cast<QWidget *>(watched);
        if(rect().contains(watchedWidget->rect()))
        {
            if(fadeVisible() && m_opacityEffect && m_opacityEffect->opacity()!=1)
            {
                switch (event->type())
                {
                case QEvent::MouseMove:
                case QEvent::MouseButtonPress:
                case QEvent::MouseButtonDblClick:
                case QEvent::MouseButtonRelease:
                case QEvent::KeyPress:
                case QEvent::KeyRelease:
                    return true;
                default:
                    break;
                }
            }
        }

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
        m_parentWidget = parentWidget();
        if (m_parentWidget != nullptr)
        {
            adjustGeometry(m_parentWidget->size());
        }
    }
    QWidget::changeEvent(event);
}

bool KSLockerDemo::fadeVisible()
{
    return m_fadeVisible;
}

void KSLockerDemo::fadeIn()
{
    if (m_fadeVisible)
    {
        return;
    }

    KLOG_DEBUG() << "fade in...";

    m_fadeVisible = true;

    if (m_enableAnimation)
    {
        m_animation->setDuration(1300);
        m_animation->setDirection(QAbstractAnimation::Forward);
        m_animation->start();
    }
    else
    {
        m_opacityEffect->setOpacity(1);
    }
}

void KSLockerDemo::fadeOut()
{
    if (!m_fadeVisible)
    {
        return;
    }

    KLOG_DEBUG() << "fade out...";

    m_fadeVisible = false;

    if (m_enableAnimation)
    {
        m_animation->setDuration(50);
        m_animation->setDirection(QAbstractAnimation::Backward);
        m_animation->start();
    }
    else
    {
        m_opacityEffect->setOpacity(0);
    }
}

void KSLockerDemo::init()
{
    setAttribute(Qt::WA_TranslucentBackground);

    //　设置透明效果
    m_opacityEffect = new QGraphicsOpacityEffect(this);
    m_opacityEffect->setOpacity(0);
    setGraphicsEffect(m_opacityEffect);

    // 渐出动画
    if (m_enableAnimation)
    {
        m_animation = new QPropertyAnimation(this);
        m_animation->setTargetObject(m_opacityEffect);
        m_animation->setPropertyName("opacity");
        m_animation->setStartValue(0);
        m_animation->setEndValue(1);
        connect(m_animation, &QPropertyAnimation::finished, [this]() {

        });
    }

    // 给父控件安装事件监控,更新窗口大小
    if (m_parentWidget != nullptr)
    {
        adjustGeometry(m_parentWidget->size());
    }

    connect(ui->pushButton, &QPushButton::clicked, [this]() {
        emit authenticationPassed();
    });
}

void KSLockerDemo::adjustGeometry(const QSize &size)
{
    QRect rect(0, 0, size.width(), size.height());
    setGeometry(rect);
}