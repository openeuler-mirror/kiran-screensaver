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

#ifndef KIRAN_SCREENSAVER_SRC_SCREEN_MANAGER_KS_SCREEN_MANAGER_H_
#define KIRAN_SCREENSAVER_SRC_SCREEN_MANAGER_KS_SCREEN_MANAGER_H_

#include <QMap>
#include <QObject>
#include <QImage>
#include "ks-interface.h"

class KSWindow;
class QScreen;
class QGSettings;
class KSFade;
class KSScreensaver;
class QStateMachine;
class KSPrefs;
class KSPluginInterface;
class KSLockerInterface;

//TODO: 还未处理解锁框跟着指针移动的情况
//TODO: 暂时只在主屏上显示和模糊
class KSScreenManager : public QObject,public KSInterface
{
    Q_OBJECT
public:
    KSScreenManager(KSFade* fade,
                    QObject* parent = nullptr);
    ~KSScreenManager();

    bool init();
    void authenticationPassed() override;

    bool eventFilter(QObject* watched, QEvent* event) override;

public:
    // 激活状态 (标志背景窗口和屏保是否被创建)
    bool setActive(bool visible);
    bool getActive();

    // 锁定框激活状态 (标志锁定框时候被创建)
    bool setLockActive(bool lockActive);
    bool getLockActive() const;

    // 锁定框是否可见
    void setLockVisible(bool lockVisible);
    bool getLockVisible();

signals:
    void sigReqDeactivated();

private:
    bool eventFilterActivate(QObject* watched,QEvent* event);
    bool eventFilterCurrentWindowResize(QObject* watched,QEvent* event);

    void updateCurrentSubWindowGeometry(QSize size);
    void moveContentToWindow(KSWindow* window);

    bool activate();
    void createWindows();
    KSWindow* createWindowForScreen(QScreen* screen);

    bool deactivate();
    void destroyWindows();

private slots:
    void handleScreenAdded(QScreen* screen);
    void handleScreenRemoved(QScreen* screen);

private:
    // kiran-screensaver配置项
    KSPrefs* m_prefs = nullptr;
    // 屏幕淡出接口实现
    KSFade* m_fade = nullptr;

    // 是否启用动画
    bool m_enableAnimation = false;
    // 空闲时激活锁定
    bool m_idleActivationLock = false;

    // 屏保和容器窗口是否激活
    bool m_active = false;
    // 解锁框是否激活
    bool m_lockActive = false;
    // 解锁框是否可见
    bool m_lockerVisible = false;

    // 窗口背景缓存
    QImage m_background;
    // 屏幕与背景容器窗口映射
    QMap<QScreen*, KSWindow*> m_windowMap;
    // 当前显示内容的背景窗口
    KSWindow* m_currentWindow = nullptr;
    // 屏保界面
    KSScreensaver* m_screensaver;
    // 解锁框界面
    KSPluginInterface* m_lockerPluginInterface;
    KSLockerInterface* m_lockerInterface;
};

#endif  //KIRAN_SCREENSAVER_SRC_SCREEN_MANAGER_KS_SCREEN_MANAGER_H_
