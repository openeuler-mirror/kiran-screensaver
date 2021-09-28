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
#ifndef FLOATWIDGET_H
#define FLOATWIDGET_H

#include <QWidget>
#include <QGraphicsOpacityEffect>

class QSequentialAnimationGroup;
class FloatWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(int floatingDistance READ floatingDistance WRITE setFloatingDistance)
public:
    enum FloatingDirection
    {
        DIRECTION_UP,
        DIRECTION_DOWN
    };
    FloatWidget(QWidget* parent = nullptr);
    virtual ~FloatWidget();

    // 设置父窗口锚点，在父窗口的x,y轴百分比
    void setAnchor(qreal xAxisPercentage,qreal yAxisPercentage);

    // 设置漂浮参数
    void setFloatingParameter(FloatingDirection direction,bool floatingOpacity,int durationMs,int distancePx);

    void start();
    void stop();
    void reset();

    int floatingDistance() const;
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    void initAnimation();
    void updateCurrentPosition();

public slots:
    void setFloatingDistance(int floatingDistance);

protected:
    virtual void changeEvent(QEvent *event) override;
    virtual void paintEvent(QPaintEvent *event) override;

private:
    qreal m_xAxisPercent = 0.5;
    qreal m_yAxisPercent = 0.5;

    FloatingDirection m_floatingDirection = DIRECTION_UP;
    int m_durationMs = 200;
    int m_distancePx = 30;
    bool m_enableFloatingOpacity = true;

    int m_floatingDistance = 0;

    QGraphicsOpacityEffect* m_opacityEffect = nullptr;

    QSequentialAnimationGroup* m_floatingAnimation = nullptr;
};

#endif // FLOATWIDGET_H
