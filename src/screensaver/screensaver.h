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

#ifndef KIRAN_SCREENSAVER_SRC_SCREENSAVER_SCREENSAVER_H_
#define KIRAN_SCREENSAVER_SRC_SCREENSAVER_SCREENSAVER_H_

#include <QWidget>

QT_BEGIN_NAMESPACE

QT_END_NAMESPACE

class QGraphicsOpacityEffect;
class QStateMachine;
class QState;

namespace Kiran
{
namespace ScreenSaver
{


namespace Ui
{
class Screensaver;
}  // namespace Ui

class GraphicsGlowEffect;
class FloatLabel;
class Screensaver : public QWidget
{
    Q_OBJECT
public:
    explicit Screensaver(bool enableAnimation,
                         QWidget* parent = nullptr);
    ~Screensaver() override;

    // 遮盖状态,是否覆盖父窗口内容
    bool maskState();
    void setMaskState(bool maskState);

private:
    void init();
    void initGraphicsEffect();
    void setupStateMachine();
    void updateStateProperty();

signals:
    void masking();
    void unmasking();

protected:
    void resizeEvent(QResizeEvent* event) override;

private slots:
    void startUpdateTimeDateTimer();

private:
    Ui::Screensaver* ui;
    FloatLabel* m_floatingLabel = nullptr;

    bool m_masked = true;
    bool m_enableAnimation = false;

    QStateMachine* m_stateMachine = nullptr;
    QState* m_maskState = nullptr;
    QState* m_unMaskState = nullptr;

    GraphicsGlowEffect* m_opacityEffect = nullptr;
};

}  // namespace ScreenSaver
}  // namespace Kiran
#endif  //KIRAN_SCREENSAVER_SRC_SCREENSAVER_SCREENSAVER_H_
