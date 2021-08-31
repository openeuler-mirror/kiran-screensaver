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

    //TODO:正式发布应加入此标志位
    //setWindowFlag(Qt::BypassWindowManagerHint);

    if (m_enableAnimation)
    {
        m_blurAnimation = new QPropertyAnimation(this);
        m_blurAnimation->setTargetObject(this);
        m_blurAnimation->setPropertyName("blurOpacity");
        m_blurAnimation->setStartValue(0);
        m_blurAnimation->setEndValue(1);
        m_blurAnimation->setDuration(500);
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

void KSWindow::setBackground(const QPixmap &pixmap)
{
    m_background = pixmap;
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

        QSize newPixbufSize;
        newPixbufSize.setWidth(qFloor(imageSize.width() * factor + 0.5));
        newPixbufSize.setHeight(qFloor(imageSize.height() * factor + 0.5));

        QPixmap scaledPixmap = m_background.scaled(newPixbufSize, Qt::KeepAspectRatio, Qt::FastTransformation);

        QImage scaledBlurImage = scaledPixmap.toImage();
        qt_blurImage(scaledBlurImage, 35.0, false);

        m_scaledBackground = scaledPixmap;
        m_blurScaledBackground = QPixmap::fromImage(scaledBlurImage);
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
        QRect drawTargetRect((imageSize.width() - windowSize.width()) / -2,
                             (imageSize.height() - windowSize.height()) / -2,
                             imageSize.width(),
                             imageSize.height());

        if (m_blurOpacity != 1)
        {
            painter.drawPixmap(drawTargetRect, m_scaledBackground, m_scaledBackground.rect());
        }

        if (m_blurOpacity > 0)
        {
            painter.save();
            painter.setOpacity(m_blurOpacity);
            painter.drawPixmap(drawTargetRect, m_blurScaledBackground, m_blurScaledBackground.rect());
            painter.restore();
        }
    }
    QWidget::paintEvent(event);
}

void KSWindow::enterEvent(QEvent *event)
{
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

void KSWindow::startBlur()
{
    if (m_enableAnimation)
    {
        m_blurAnimation->start();
    }
    else
    {
        m_blurOpacity = 1;
        update();
    }
}

void KSWindow::resetBlur()
{
    if (m_enableAnimation)
    {
        m_blurAnimation->stop();
    }
    m_blurOpacity = 0;
    update();
}
