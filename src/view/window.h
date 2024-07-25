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

#ifndef KIRAN_SCREENSAVER_SRC_VIEW_WINDOW_H_
#define KIRAN_SCREENSAVER_SRC_VIEW_WINDOW_H_

#include <QTimer>
#include <QWidget>

class QPropertyAnimation;
class QTimer;
namespace Kiran
{
namespace ScreenSaver
{
class ScreenSaver;
class Window : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(qreal blurOpacity READ blurOpacity WRITE setBlurOpacity)
public:
    explicit Window(bool enableAnimation,
                      QScreen* screen = nullptr);
    ~Window() override;

    // 背景模糊
    qreal blurOpacity();
    void setBlurOpacity(qreal blurOpacity);

    void setBlurBackground(bool blur);
    bool getBlurBackground();

    // 设置绑定屏幕
    void setScreen(QScreen* screen);

    // 设置显示背景图
    void setBackground(const QImage& background);

    void raiseDelay();
signals:
    void mouseEnter();

private:
    void handleScreenGeometryChanged(const QRect& geometry);

protected:
    void resizeEvent(QResizeEvent* event) override;
    void paintEvent(QPaintEvent* event) override;
    void enterEvent(QEvent* event) override;
    void timerEvent(QTimerEvent* event) override;

private:
    int m_animationDelayTimerID = 0;

    QScreen* m_screen = nullptr;
    bool m_enableAnimation = false;

    bool m_blurBackground = false;
    qreal m_blurOpacity = 0;
    QPropertyAnimation* m_blurAnimation = nullptr;

    QImage m_background;
    QImage m_scaledBackground;
    QImage m_blurScaledBackground;

    QTimer* m_delayRaiseTimer = nullptr;
};
}  // namespace ScreenSaver
}  // namespace Kiran
#endif  //KIRAN_SCREENSAVER_SRC_VIEW_WINDOW_H_
