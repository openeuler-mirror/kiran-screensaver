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
#include <qt5-log-i.h>
#include "ks-idle-watcher-mate.h"
#include "ks-fade.h"
#include "ks-prefs.h"
#include "ks_kiran_adaptor.h"
#include "ks_mate_adaptor.h"
#include "ks-listener.h"
#include "ks-grab.h"
#include <QApplication>

 KSManager::KSManager()
{

}

KSManager::~KSManager()
{
    ///NOTE: 删除nullptr无影响
    delete m_prefs;
    delete m_fade;
    delete m_kiranAdaptor;
    delete m_mateAdaptor;
    delete m_listener;
}

#include "ks-window.h"

KSWindow* window = nullptr;

bool KSManager::init()
{
    ///test
    QTimer::singleShot(1000,[this](){
        window = new KSWindow;
        window->setScreen(qApp->primaryScreen());
        window->showScreenSaver("/home/lxh/rpmbuild/RPMS/x86_64/usr/libexec/xscreensaver/pacman");
        window->show();
    });
    QTimer::singleShot(30000,[this](){
        delete window;
    });

    m_grab = KSGrab::getInstance();

    m_fade = new KSFade;

    m_prefs = new KSPrefs;
    if(!m_prefs->init())
    {
        return false;
    }

    if(!initDBusListener())
    {
        return false;
    }

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
            this,&KSManager::handleListenerActiveChaneged,
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
    KSIdleWatcher* idleWatcher = new KSIdleWatcherMate;
    idleWatcher->setEnabled(true);
    idleWatcher->setActive(true);
    if(!idleWatcher->init())
    {
        KLOG_ERROR() << "can't init idle watcher";
        return false;
    }
    connect(idleWatcher,&KSIdleWatcher::idleChanged,this,&KSManager::handleIdleChanged,Qt::DirectConnection);
    connect(idleWatcher,&KSIdleWatcher::idleNoticeChanged,this,&KSManager::handleIdleNoticeChanged,Qt::DirectConnection);
    return true;
}

void KSManager::handleIdleChanged(bool idle, bool& handled)
{
    KLOG_DEBUG("handle idle changed: %s",idle? "idle" : "not idle");

    //空闲
        //抓取鼠标键盘至root窗口上,避免按压键盘或鼠标影响桌面会话中的内容
        //判断是否允许屏保出现,显示窗口
    //非空闲
        //激活屏保
    if(  m_listener->isInhibited() )
    {

    }
}

void KSManager::handleIdleNoticeChanged(bool isEffect, bool& handled)
{
    KLOG_DEBUG() << "handle idle notice changed: " << isEffect;

    ///TODO:空闲时激活配置项目，从KSPrefs中取出
    bool activationEnabled = true;

    ///TODO:是否被抑制，从KSListener中取出
    bool inhibited = false;

    ///TODO:屏保是否显示出来
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
            m_grab->releaseGrab();
            m_fade->reset();
            handled = true;
        }
    }
}

void KSManager::handleListenerActiveChaneged(bool active, bool& handled)
{

}

void KSManager::handleListenerLock()
{

}
