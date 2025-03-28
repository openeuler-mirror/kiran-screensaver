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

#include "manager.h"
#include "fade.h"
#include "grab.h"
#include "idle-watcher-factory.h"
#include "kiran_adaptor.h"
#include "listener.h"
#include "prefs.h"
#include "screen-manager.h"
#include "window.h"

#include <qt5-log-i.h>
#include <QApplication>

using namespace Kiran::ScreenSaver;

Manager::Manager()
{
}

Manager::~Manager()
{
    // delete nullptr无影响
    delete m_screenManager;
    delete m_fade;
    delete m_kiranAdaptor;
    delete m_listener;
}

bool Manager::init()
{
    m_prefs = Prefs::getInstance();
    if (!m_prefs->init())
    {
        KLOG_ERROR() << "kiran screensaver prefs init failed!";
        return false;
    }

    m_grab = Grab::getInstance();

    m_fade = new Fade;

    // 屏幕管理类
    m_screenManager = new ScreenManager(m_fade);
    m_screenManager->init();

    connect(m_screenManager, &ScreenManager::sigReqDeactivated, [this]()
            { m_listener->setActiveStatus(false); });

    // 初始化DBus服务
    if (!initDBusListener())
    {
        return false;
    }

    // 初始化空闲监控类
    if (!initIdleWatcher())
    {
        return false;
    }

    return true;
}

// 注册DBus服务
bool Manager::initDBusListener()
{
    m_listener = new Listener();
    if (!m_listener->init())
    {
        KLOG_ERROR() << "dbus listener init failed!";
        return false;
    }

    connect(m_listener, &Listener::sigActiveChanged,
            this, &Manager::onListenerActiveChanged,
            Qt::DirectConnection);

    connect(m_listener, &Listener::sigLock,
            this, &Manager::handleListenerLock);

    m_kiranAdaptor = new KiranAdaptor(m_listener);
    QDBusConnection sessionConnection = QDBusConnection::sessionBus();
    if (!sessionConnection.registerService("com.kylinsec.Kiran.ScreenSaver"))
    {
        KLOG_ERROR() << "session bus can't register service:" << sessionConnection.lastError();
        return false;
    }
    if (!sessionConnection.registerObject("/com/kylinsec/Kiran/ScreenSaver", "com.kylinsec.Kiran.ScreenSaver", m_listener))
    {
        KLOG_ERROR() << "can't register object:" << sessionConnection.lastError();
        return false;
    }
    if (!sessionConnection.registerService("org.mate.ScreenSaver"))
    {
        KLOG_ERROR() << "session bus can't register service:" << sessionConnection.lastError().message();
        return false;
    }
    if (!sessionConnection.registerObject("/org/mate/ScreenSaver", m_listener))
    {
        KLOG_ERROR() << "can't register object:" << sessionConnection.lastError();
        return false;
    }

    return true;
}

bool Manager::initIdleWatcher()
{
    m_idleWatcher = IdleWatcherFactory::createIdleWatcher();
    if (m_idleWatcher == nullptr)
    {
        return false;
    }

    if (!m_idleWatcher->init())
    {
        KLOG_ERROR() << "can't init idle watcher";
        return false;
    }

    connect(m_idleWatcher, &IdleWatcher::idleChanged, this, &Manager::onWatcherIdleChanged, Qt::DirectConnection);
    connect(m_idleWatcher, &IdleWatcher::idleNoticeChanged, this, &Manager::onWatcherIdleNoticeChanged, Qt::DirectConnection);

    processIdleTriggerChanged();
    connect(m_prefs, &Prefs::idleActivationLockChanged, this, &Manager::processIdleTriggerChanged);
    connect(m_prefs, &Prefs::idleActivationScrensaverChanged, this, &Manager::processIdleTriggerChanged);
    return true;
}

void Manager::onWatcherIdleChanged(bool idle, bool& handled)
{
    KLOG_DEBUG("handle idle changed: %s", idle ? "idle" : "not idle");

    // 设置空闲状态到KSListener,KSManager在KSListener发出的信号中进行相关操作
    bool res = m_listener->setSessionIdle(idle);

    if (!res)
    {
        KLOG_DEBUG() << "can't set listener session idle" << idle;
    }
    else
    {
        handled = true;
    }
}

// 处理KSIdleWatcher发来的空闲预告消息
// 如果是空闲预告 isEffect true,将开启屏幕淡出,抓取鼠标键盘等输入设备输入(因用户可通过按键或鼠标移动取消进入空闲,避免误输入的情况)
// 如果是空闲预告 isEffect false,可能两种情况:
//     1. not idle -> idle notice 状态下被取消进入空闲
//     2. not idle -> idle notice(true) -> idle -> idle notice(false) 已进入空闲状态
void Manager::onWatcherIdleNoticeChanged(bool isEffect, bool& handled)
{
    KLOG_DEBUG() << "handle idle notice changed: " << isEffect;

    if (isEffect)  /// 空闲预告
    {
        // 抓取鼠标键盘输入输出（屏幕淡出时，可通过点击鼠标或敲击键盘取消，这个取消过程的响应不应该被传入桌面）,是否成功
        if (!m_grab->grabOffscreen(true))
        {
            KLOG_WARNING() << "grab pointer and keyboard offscreen failed!";
            return;
        }

        // 电源插件后端开启了"空闲时变暗显示器"功能时，不启动空闲预告淡出动画, 避免和该功能冲突
        if (!m_prefs->getEnableDisplayIdleDimmed())
        {
            // 屏幕开始淡出
            m_fade->startAsync();
        }

        handled = true;
    }
    else  /// 取消空闲预告
    {
        // 屏保和内容窗口已显示出来,此时屏幕淡出效果以及输入设备的抓取由KSScreenManager负责操作
        if (m_screenManager->getActive())
        {
            KLOG_DEBUG() << "screen manager active,skipping fade cancelation!";
        }
        else
        {
            // 屏保未显示出来,正当前正处于"no idle"、"idle-notice"、"idle"第二步和第三步之间的位置
            // 此时发出的预告则是取消掉预告通知

            // 复位屏幕的淡出效果
            m_fade->reset();

            // 取消预告通知,释放抓取输入设备
            QTimer::singleShot(500, [this]()
                               { m_grab->releaseGrab(); });
        }
        handled = true;
    }
}

// 激活屏保
// 1. 空闲触发 idle notice -> idle -> listener active
// 2. 外部触发 listener activate/deactivate
// 3. 外部触发 listener lock -> listener active -> lock
void Manager::onListenerActiveChanged(bool active, bool& handled)
{
    bool isSessionIdle = m_listener->getSessionIdle();

    // 会话空闲激活配置检查，不允许空闲什么也不激活
    if (isSessionIdle && active &&
        !m_prefs->getIdleActivationScreensaver() &&
        !m_prefs->getIdleActivationLock())
    {
        KLOG_WARNING("try to activate but idle activation is disabled");
        return;
    }

    // 改变屏幕管理状态
    if (!m_screenManager->setActive(active))
    {
        KLOG_WARNING() << "can't set screen manager active status:" << active;
        return;
    }

    handled = true;

    // 会话空闲激活，根据配置确认是否激活屏保/锁定
    if (isSessionIdle && active)
    {
        auto screensaverEnable = m_prefs->getIdleActivationScreensaver();
        if (screensaverEnable)
        {
            m_screenManager->setScreenSaverActive(true);
        }

        auto lockEnable = m_prefs->getIdleActivationLock();
        if (lockEnable)
        {
            m_screenManager->setLockActive(lockEnable);
            // 如果屏保可见的话，保持屏保可见。
            m_screenManager->setLockVisible(!screensaverEnable);
        }
    }
    else if (active)  // DBus激活/或内部调用触发,应直接触发屏保
    {
        m_screenManager->setScreenSaverActive(true);
    }

    if (m_idleWatcher->getEnabled())
    {
        if (!m_idleWatcher->setIdleDetectionActive(!active))
        {
            KLOG_ERROR() << "can't set idle watcher active status:" << !active;
        }
    }

    return;
}

// KSListener收到外部DBus调用"Lock"方法处理槽函数
// 通过 ScreenManager 设置锁定标志位
// 获取 KSScreenManger 屏保激活状态(窗口是否创建以及显示),若没进行显示,则应该设置KSListener屏保状态(在KSListener信号槽中进行激活KSScreenManager)
void Manager::handleListenerLock()
{
    if (m_screenManager->getActive())
    {
        // 屏保已被激活,显示锁定框
        if (m_screenManager->getLockActive())
        {
            m_screenManager->setLockActive(true);
        }
    }
    else
    {
        // 屏保未被激活,通过Listener设置激活状态,触发屏保显示
        if (m_listener->setActiveStatus(true))
        {
            // 显示出锁定框
            m_screenManager->setLockActive(true);
            m_screenManager->setLockVisible(true);
        }
        else
        {
            KLOG_DEBUG() << "can't activate screensaver and locker";
        }
    }
}

void Manager::processIdleTriggerChanged()
{
    auto idleTriggerLock = m_prefs->getIdleActivationLock();
    auto idleTriggerScreensaver = m_prefs->getIdleActivationScreensaver();

    // 启用空闲触发，并且当前IdleWatcher是禁用状态
    if ((idleTriggerLock || idleTriggerScreensaver) && !m_idleWatcher->getEnabled())
    {
        m_idleWatcher->setEnabled(true);
        m_idleWatcher->setIdleDetectionActive(true);
    }  // 未启用空闲触发，并且当前IdleWatcher是启用状态
    else if (!idleTriggerLock && !idleTriggerScreensaver && m_idleWatcher->getEnabled())
    {
        m_idleWatcher->setEnabled(false);
        m_idleWatcher->setIdleDetectionActive(false);
    }
}
