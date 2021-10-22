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

class KSScreenManager : public QObject,public KSInterface
{
    Q_OBJECT
public:
    KSScreenManager(KSFade* fade,
                    QObject* parent = nullptr);
    ~KSScreenManager();

    //　初始化加载相关插件
    bool init();

    //　提供接口给解锁框插件调用,传达解锁框认证成功的消息
    void authenticationPassed() override;

    // 全局事件过滤器
    // 处理解锁框激活或非激活
    // 处理窗口大小Resize事件
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
    // 向外发送deactivate请求
    void sigReqDeactivated();

private:
    // 事件过滤激活或非激活解锁框事件
    bool eventFilterActivate(QObject* watched,QEvent* event);
    // 事件过滤当前窗口resize事件
    bool eventFilterCurrentWindowResize(QObject* watched,QEvent* event);

    // 更新当前窗子窗口(屏保和解锁款)大小
    void updateCurrentSubWindowGeometry(QSize size);
    // 移动屏保和解锁框到该窗口上
    void moveContentToWindow(KSWindow* window);

    bool activate();
    void createWindows();

    KSWindow* createWindowForScreen(QScreen* screen);
    void deleteWindowForScreen(QScreen* screen);

    bool deactivate();
    void destroyWindows();

    void setBackgroundWindowBlured(KSWindow* window);
private slots:
    void handleScreenAdded(QScreen* screen);
    void handleScreenRemoved(QScreen* screen);
    void handleWindowMouseEnter();

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
