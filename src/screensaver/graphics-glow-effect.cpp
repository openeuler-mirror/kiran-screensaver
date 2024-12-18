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

#include "graphics-glow-effect.h"

QT_BEGIN_NAMESPACE
extern Q_WIDGETS_EXPORT void qt_blurImage(QPainter *p, QImage &blurImage, qreal radius, bool quality, bool alphaOnly, int transposed = 0);
QT_END_NAMESPACE

using namespace Kiran::ScreenSaver;

GraphicsGlowEffect::GraphicsGlowEffect(QObject *parent) :
    QGraphicsEffect(parent),
    m_xOffset(0),
    m_yOffset(0),
    m_distance(4.0),
    m_blurRadius(10.0),
    m_color(0, 0, 0, 80)
{
}

// 将控件的扩散效果绘制到指定的painter上。
void GraphicsGlowEffect::draw(QPainter *painter)
{
    // 此处直接调用drawSource不会绘制source非布局中的内容，例如Screensaver中的FloatLabel
#if 0
    // if nothing to show outside the item, just draw source
    if ((blurRadius() + distance()) <= 0) {
        drawSource(painter);
        return;
    }
#endif

    PixmapPadMode mode = QGraphicsEffect::PadToEffectiveBoundingRect;
    QPoint offset;
    const QPixmap sourcePx = sourcePixmap(Qt::DeviceCoordinates, &offset, mode);

    // return if no source
    if (sourcePx.isNull()) {
        return;
    }

    qreal restoreOpacity = painter->opacity();
    painter->setOpacity(m_opacity);

    // 模糊半径以及distance相加小于等于0，不做高斯模糊，只做透明度处理
    // 直接绘制QGraphicsEffectSource中取来的Pixmap
    if ((blurRadius() + distance()) <= 0) {
        painter->drawPixmap(offset,sourcePx);
        return;
    }

    // save world transform
    QTransform restoreTransform = painter->worldTransform();
    painter->setWorldTransform(QTransform());

    // Calculate size for the background image
    QSize scaleSize(sourcePx.size().width() + 2 * distance(),
                    sourcePx.size().height() + 2 * distance());

    QImage tmpImg(scaleSize, QImage::Format_ARGB32_Premultiplied);
    QPixmap scaled = sourcePx.scaled(scaleSize);
    tmpImg.fill(0);
    QPainter tmpPainter(&tmpImg);
    tmpPainter.setCompositionMode(QPainter::CompositionMode_Source);
    tmpPainter.drawPixmap(QPointF(-distance(), -distance()), scaled);
    tmpPainter.end();

    // blur the alpha channel
    QImage blurred(tmpImg.size(), QImage::Format_ARGB32_Premultiplied);
    blurred.fill(0);
    QPainter blurPainter(&blurred);
    qt_blurImage(&blurPainter, tmpImg, blurRadius(), false, true);
    blurPainter.end();

    tmpImg = blurred;

    // blacken the image...
    tmpPainter.begin(&tmpImg);
    tmpPainter.setCompositionMode(QPainter::CompositionMode_SourceIn);
    tmpPainter.fillRect(tmpImg.rect(), color());
    tmpPainter.end();

    // draw the blurred shadow...
    painter->drawImage(offset, tmpImg);

    // draw the actual pixmap...
    painter->drawPixmap(offset, sourcePx, QRectF());

    // restore world transform
    painter->setWorldTransform(restoreTransform);
    painter->setOpacity(restoreOpacity);
}

// 获取有效的边界区域
QRectF GraphicsGlowEffect::boundingRectFor(const QRectF &rect) const
{
    qreal delta = blurRadius() + distance();
    return rect.united(rect.adjusted(-delta - xOffset(), -delta - yOffset(), delta - xOffset(), delta - yOffset()));
}