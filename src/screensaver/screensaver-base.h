/**
 * Copyright (c) 2020 ~ 2025 KylinSec Co., Ltd.
 * kiran-screensaver is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 *
 * Author:     liuxinhao <liuxinhao@kylinsec.com.cn>
 */
#pragma once
#include <QWidget>

class QStateMachine;
class QState;
class QResizeEvent;
class QPaintEvent;
namespace Kiran
{
namespace ScreenSaver
{
class GraphicsGlowEffect;
class ScreensaverBase : public QWidget
{
    Q_OBJECT
public:
    explicit ScreensaverBase(bool animated, QWidget* parent = nullptr);
    virtual ~ScreensaverBase();

    // 遮盖状态,是否覆盖父窗口内容
    bool isMasked();
    void setMaskState(bool maskState);

    static bool isSupported(const QString& name) { return false; };

signals:
    void updateTime();
    void masking();
    void unmasking();

protected:
    void updateStateProperty();
    virtual void resizeEvent(QResizeEvent* event) override;

private:
    void init();
    void initStateMachine();

protected slots:
    // 触发更新时钟, 根据当前刷新时间，计算下次刷新时间(外部触发)
    void startUpdateTimeDateTimer();

private:
    bool m_masked = true;
    bool m_enableAnimation = false;
    QStateMachine* m_stateMachine = nullptr;
    QState* m_maskState = nullptr;
    QState* m_unmaskState = nullptr;
    GraphicsGlowEffect* m_opacityEffect = nullptr;
};
}  // namespace ScreenSaver
}  // namespace Kiran
