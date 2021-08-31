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

#ifndef KIRAN_SCREENSAVER_SRC_SCREENSAVER_KS_SCREENSAVER_H_
#define KIRAN_SCREENSAVER_SRC_SCREENSAVER_KS_SCREENSAVER_H_

#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui
{
class KSScreensaver;
}
QT_END_NAMESPACE

class KiranGraphicsGlowEffect;
class QStateMachine;
class QState;
class KSScreensaver : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(bool active READ active WRITE setActive)
public:
    explicit KSScreensaver(QWidget *parent = nullptr);
    ~KSScreensaver() override;

    bool active();
    void setActive(bool active);

    bool eventFilter(QObject* watched, QEvent* event) override;

private:
    void initGraphicsEffect();
    void setupStateMachine();
    void adjustGeometry(const QSize& size);
    void updateStateProperty();

protected:
    void resizeEvent(QResizeEvent* event) override;
    void changeEvent(QEvent *event) override;

signals:
    void activation();
    void inactivation();

private:
    Ui::KSScreensaver *ui;
    bool m_isActive = true;
    QWidget* m_parentWidget = nullptr;
    QStateMachine* m_stateMachine = nullptr;
    QState* m_activeState = nullptr;
    QState* m_unactiveState = nullptr;
    KiranGraphicsGlowEffect* m_opacityEffect = nullptr;
};

#endif  //KIRAN_SCREENSAVER_SRC_SCREENSAVER_KS_SCREENSAVER_H_
