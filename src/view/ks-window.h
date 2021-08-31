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

#ifndef KIRAN_SCREENSAVER_SRC_VIEW_KS_WINDOW_H_
#define KIRAN_SCREENSAVER_SRC_VIEW_KS_WINDOW_H_

#include <QWidget>

class KSScreenSaver;
class QPropertyAnimation;
class KSWindow : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(qreal blurOpacity READ blurOpacity WRITE setBlurOpacity)
public:
    explicit KSWindow(bool enableAnimation,
                      QScreen* screen = nullptr);
    ~KSWindow() override;

    // 背景模糊
    qreal blurOpacity();
    void setBlurOpacity(qreal blurOpacity);

    void startBlur();
    void resetBlur();

    // 设置绑定屏幕
    void setScreen(QScreen* screen);

    // 设置显示背景图
    void setBackground(const QPixmap& pixmap);

    void setScreenSaverVisible(bool visible);
    bool getScreenSaverVisible();

private:
    void handleScreenGeometryChanged(const QRect& geometry);

protected:
    void resizeEvent(QResizeEvent* event) override;
    void paintEvent(QPaintEvent* event) override;
    void enterEvent(QEvent* event) override;

private:
    QScreen* m_screen = nullptr;
    bool m_enableAnimation = false;
    QPixmap m_background;
    QPixmap m_scaledBackground;
    QPixmap m_blurScaledBackground;
    KSScreenSaver* m_screensaver = nullptr;
    QPropertyAnimation* m_blurAnimation = nullptr;
    qreal  m_blurOpacity = 0;
};

#endif  //KIRAN_SCREENSAVER_SRC_VIEW_KS_WINDOW_H_
