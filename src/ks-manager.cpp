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

#include "ks-manager.h"
#include "ks-fade.h"
#include "ks-grab.h"
#include "ks-idle-watcher-factory.h"
#include "ks-listener.h"
#include "ks-prefs.h"
#include "ks-screen-manager.h"
#include "ks-window.h"
#include "ks_kiran_adaptor.h"
#include "ks_mate_adaptor.h"

#include <qt5-log-i.h>
#include <QApplication>

 KSManager::KSManager()
{

}

KSManager::~KSManager()
{
    // delete nullptr无影响
    delete m_screenManager;
    delete m_prefs;
    delete m_fade;
    delete m_kiranAdaptor;
    delete m_mateAdaptor;
    delete m_listener;
}

bool KSManager::init()
{
    // 加载配置项
    m_prefs = new KSPrefs;
    if(!m_prefs->init())
    {
        KLOG_ERROR() << "kiran screensaver prefs init failed!";
        return false;
    }

    // 抓取类
    m_grab = KSGrab::getInstance();

    // 屏幕淡出功能
    m_fade = new KSFade;

    // 屏幕管理类
    m_screenManager = new KSScreenManager(nullptr, m_fade);

    // 初始化DBus服务
    if(!initDBusListener())
    {
        return false;
    }

    //初始化空闲监控类
    if(!initIdleWatcher())
    {
        return false;
    }

    return true;
}

//注册DBus服务
bool KSManager::initDBusListener()
{
    m_listener = new KSListener();
    if( !m_listener->init() )
    {
        KLOG_ERROR() << "dbus listener init failed!";
        return false;
    }

    connect(m_listener,&KSListener::sigActiveChanged,
            this,&KSManager::handleListenerActiveChanged,
            Qt::DirectConnection);

    connect(m_listener,&KSListener::sigLock,
            this,&KSManager::handleListenerLock);

    m_kiranAdaptor = new KSKiranAdaptor(m_listener);
    m_mateAdaptor = new KSMateAdaptor(m_listener);

    QDBusConnection sessionConnection = QDBusConnection::sessionBus();
    if(!sessionConnection.registerService("com.kylinsec.ScreenSaver"))
    {
        KLOG_ERROR() << "session bus can't register service:" << sessionConnection.lastError();
        return false;
    }
    if(!sessionConnection.registerObject("/com/kylinsec/Kiran/ScreenSaver","com.kylinsec.Kiran.ScreenSaver",m_listener))
    {
        KLOG_ERROR() << "can't register object:" << sessionConnection.lastError();
        return false;
    }
    if(!sessionConnection.registerService(  "org.mate.ScreenSaver"))
    {
        KLOG_ERROR() << "session bus can't register service:" << sessionConnection.lastError().message();
        return false;
    }
    if(!sessionConnection.registerObject("/org/mate/ScreenSaver",m_listener))
    {
        KLOG_ERROR() << "can't register object:" << sessionConnection.lastError();
        return false;
    }

    return true;
}

bool KSManager::initIdleWatcher()
{
    m_idleWatcher = KSIdleWatcherFactory::createIdleWatcher();
    if(m_idleWatcher == nullptr)
    {
        return false;
    }

    if(!m_idleWatcher->init())
    {
        KLOG_ERROR() << "can't init idle watcher";
        return false;
    }

    connect(m_idleWatcher,&KSIdleWatcher::idleChanged,this,&KSManager::handleIdleChanged,Qt::DirectConnection);
    connect(m_idleWatcher,&KSIdleWatcher::idleNoticeChanged,this,&KSManager::handleIdleNoticeChanged,Qt::DirectConnection);

    /// test
    m_idleWatcher->setEnabled(true);
    m_idleWatcher->setIdleDetectionActive(true);

    return true;
}

void KSManager::handleIdleChanged(bool idle, bool& handled)
{
    KLOG_DEBUG("handle idle changed: %s",idle? "idle" : "not idle");

    // 设置空闲状态到KSListener,KSManager在KSListener发出的信号中进行相关操作
    bool res = m_listener->setSessionIdle(idle);

    if(!res)
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
void KSManager::handleIdleNoticeChanged(bool isEffect, bool& handled)
{
    KLOG_DEBUG() << "handle idle notice changed: " << isEffect;

    // 空闲时激活锁定配置项
    bool activationEnabled = m_prefs->getIdleActivationLock();

    // 从KSListener取出是否被抑制状态
    bool inhibited = m_listener->isInhibited();;

    // 屏保是否已被激活
    bool isActivate = false;

    if( isEffect ) ///空闲预告
    {
        ///空闲时允许激活屏幕保护 且 未被抑制
        if(activationEnabled && !inhibited)
        {
            //抓取鼠标键盘输入输出（屏幕淡出时，可通过点击鼠标或敲击键盘取消，这个取消过程的相应不应该被传入桌面）,是否成功
            m_grab->grabOffscreen(true);
            ///开始屏幕淡入淡出
            m_fade->startAsync();
            handled = true;
        }
    }
    else ///取消空闲预告
    {
        ///屏保未显示出来,正当前正处于"no idle"、"idle-notice"、"idle"第二步和第三步之间的位置,此时发出的预告则是取消掉预告通知
        ///应释放鼠标键盘的抓取,复位屏幕的淡出效果
        if( !isActivate )
        {
            m_fade->reset();
            //延迟释放抓取输入设备,避免误输入
            QTimer::singleShot(500,[this](){
                m_grab->releaseGrab();
            });
        }
        else
        {
            // 屏保已被激活,跳过淡出的取消
            KLOG_DEBUG() << "manager active,skipping fade cancelation!";
        }
        handled = true;
    }
}

// listener 屏保激活状态改变处理方法
// 当listener 屏保激活状态激活时,通过KSScreenManager激活屏保 激活成功时 关闭空闲监控
// 当listener 屏保激活状态变更为闲置时,通过KSSScreenManager关闭屏保 开启监控
void KSManager::handleListenerActiveChanged(bool active, bool& handled)
{
    // 改变屏幕管理状态
    bool bRes = m_screenManager->setActive(active);
    if( bRes )
    {
        // 改变屏幕管理状态成功,代表处理listener激活状态成功
        handled = true;
    }
    else
    {
        handled = false;
    }

    // active:   listener 激活  关闭空闲监控
    // inactive: listener 未激活 开启空闲监控
    if ( handled && m_idleWatcher->getEnabled() )
    {
        if(m_idleWatcher->setIdleDetectionActive(!active) )
        {
            KLOG_ERROR() << "can't set idle watcher active status:" << !active;
        }
    }
}

// KSListener收到外部DBus调用"Lock"方法处理槽函数
// 通过 KSScreenManager 设置锁定标志位
// 获取 KSScreenManger 屏保激活状态(窗口是否创建以及显示),若没进行显示,则应该设置KSListener屏保状态(在KSListener信号槽中进行激活KSScreenManager)
void KSManager::handleListenerLock()
{
    if(m_screenManager->getActive())
    {
        m_screenManager->setLockActive(true);
    }
    else
    {
        if( m_screenManager->setActive(true) )
        {
            m_screenManager->setLockActive(true);
        }
    }
}
