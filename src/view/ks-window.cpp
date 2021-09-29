/**
  * @Copyright (C) 2020 ~ 2021 KylinSec Co., Ltd. 
  *
  * Author:     liuxinhao <liuxinhao@kylinos.com.cn>
  *
  * This program is free software; you can redistribute it and/or modify
  * it under the terms of the GNU General Public License as published by
  * the Free Software Foundation; either version 3 of the License, or
  * (at your option) any later version.
  *
  * This program is distributed in the hope that it will be useful,
  * but WITHOUT ANY WARRANTY; without even the implied warranty of
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  * GNU General Public License for more details.
  *
  * You should have received a copy of the GNU General Public License
  * along with this program; If not, see <http: //www.gnu.org/licenses/>. 
  */

#include "ks-window.h"

#include <QBoxLayout>
#include <QPainter>
#include <QProcess>
#include <QResizeEvent>
#include <QScreen>
#include <QWindow>
#include <QtMath>

#include <qt5-log-i.h>
#include <QGraphicsBlurEffect>
#include <QPropertyAnimation>

QT_BEGIN_NAMESPACE
// qt源代码中 src/widgets/effects/qpixmapfilter.cpp中定义模糊图像的方法
Q_WIDGETS_EXPORT void qt_blurImage(QImage &blurImage, qreal radius, bool quality, int transposed = 0);
//Q_WIDGETS_EXPORT void qt_blurImage(QPainter *p, QImage &blurImage, qreal radius, bool quality, bool alphaOnly, int transposed = 0);
QT_END_NAMESPACE

KSWindow::KSWindow(bool enableAnimation, QScreen *screen)
    : QWidget(nullptr),
      m_enableAnimation(enableAnimation)

{
    setScreen(screen);
    setWindowFlags(Qt::BypassWindowManagerHint|Qt::FramelessWindowHint|Qt::CustomizeWindowHint|Qt::NoDropShadowWindowHint);

    if (m_enableAnimation)
    {
        m_blurAnimation = new QPropertyAnimation(this);
        m_blurAnimation->setTargetObject(this);
        m_blurAnimation->setPropertyName("blurOpacity");
        m_blurAnimation->setStartValue(0.0);
        /**
         *FIXME:
         *  由于qt_blurImage模糊半径过大，将会导致左侧以及下方会透明，直接不透明绘制出将会存在黑边，
         *  所以显示模糊背景时，将留点透明度，显示下层未被模糊的背景
         */
        m_blurAnimation->setEndValue(0.9);
        m_blurAnimation->setDuration(600);
    }
}

KSWindow::~KSWindow()
{
}

void KSWindow::setScreen(QScreen *screen)
{
    if (m_screen == screen)
        return;

    if (m_screen)
    {
        disconnect(m_screen, &QScreen::geometryChanged,
                   this, &KSWindow::handleScreenGeometryChanged);
    }

    if (screen)
    {
        setObjectName(QString("screen_background_%1").arg(screen->name()));
        connect(screen, &QScreen::geometryChanged,
                this, &KSWindow::handleScreenGeometryChanged);
    }
    else
    {
        setObjectName(QString("screen_background_null"));
    }

    m_screen = screen;
    if (m_screen)
        handleScreenGeometryChanged(m_screen->geometry());
}

void KSWindow::handleScreenGeometryChanged(const QRect &geometry)
{
    setGeometry(QRect(geometry.topLeft().x(), geometry.topLeft().y(), geometry.width(), geometry.height()));
}

void KSWindow::setBackground(const QImage &background)
{
    m_background = background;
}

void KSWindow::resizeEvent(QResizeEvent *event)
{
    //由原始图片拉升已获得更好的显示效果
    if (!m_background.isNull())
    {
        QSize minSize = event->size();
        QSize imageSize = m_background.size();

        qreal factor = qMax(minSize.width() / (double)imageSize.width(),
                            minSize.height() / (double)imageSize.height());

        QSize newImageSize;
        newImageSize.setWidth(qFloor(imageSize.width() * factor + 0.5));
        newImageSize.setHeight(qFloor(imageSize.height() * factor + 0.5));

        QImage backgroundScaled = m_background.scaled(newImageSize, Qt::KeepAspectRatio, Qt::FastTransformation);

        QImage backgroundBlur =  backgroundScaled;
        qt_blurImage(backgroundBlur,35,false);

        m_scaledBackground = backgroundScaled;
        m_blurScaledBackground = backgroundBlur;
    }
    QWidget::resizeEvent(event);
}

void KSWindow::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);

    // 绘制背景
    if (!m_scaledBackground.isNull())
    {
        QSize imageSize = m_scaledBackground.size();
        QSize windowSize = size();
        QPointF drawPoint((imageSize.width() - windowSize.width()) / -2.0,
                             (imageSize.height() - windowSize.height()) / -2.0);

        painter.drawImage(drawPoint,m_scaledBackground);

        if (m_blurOpacity > 0)
        {
            painter.setOpacity(m_blurOpacity);
            painter.drawImage(drawPoint,m_blurScaledBackground);
        }
    }

    QWidget::paintEvent(event);
}

void KSWindow::enterEvent(QEvent *event)
{
    emit mouseEnter();
    QWidget::enterEvent(event);
}

qreal KSWindow::blurOpacity()
{
    return m_blurOpacity;
}

void KSWindow::setBlurOpacity(qreal blurOpacity)
{
    KLOG_DEBUG() << "set blur opacity:" << blurOpacity;
    m_blurOpacity = blurOpacity;
    update();
}

void KSWindow::setBlurBackground(bool blur)
{
    if(m_blurBackground == blur)
    {
        return;
    }

    m_blurBackground = blur;

    if(m_enableAnimation)
    {
        if(m_blurBackground)
        {
            m_blurAnimation->start();
        }
        else
        {
            m_blurAnimation->stop();
            m_blurOpacity = 0;
            update();
        }
    }
    else
    {
        if(m_blurBackground)
        {
            m_blurOpacity = 1;
        }
        else
        {
            m_blurOpacity = 0;
        }
        update();
    }
}

bool KSWindow::getBlurBackground()
{
    return m_blurBackground;
}