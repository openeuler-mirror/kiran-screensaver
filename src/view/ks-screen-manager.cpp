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
#include "ks-screen-manager.h"
#include "ks-fade.h"
#include "ks-prefs.h"
#include "ks-window.h"
#include "locker/ks-locker-demo.h"
#include "screensaver/ks-screensaver.h"

#include <qt5-log-i.h>
#include <QApplication>
#include <QGSettings>
#include <QKeyEvent>
#include <QScreen>
#include <QStateMachine>
#include <QTimer>

KSScreenManager::KSScreenManager(KSPrefs *prefs,
                                 KSFade *fade,
                                 QObject *parent)
    : QObject(parent),
      m_fade(fade),
      m_prefs(prefs)
{
    if (m_prefs != nullptr)
    {
        m_enableAnimation = m_prefs->getEnableAnimation();
    }

    qApp->installEventFilter(this);
}

KSScreenManager::~KSScreenManager()
{
    destroyWindows();
}

bool KSScreenManager::setActive(bool active)
{
    bool res = false;

    if (active)
    {
        res = activate();
    }
    else
    {
        res = deactivate();
    }

    KLOG_DEBUG() << "set screen manager:" << (active?"activate":"inactivate") << ":" << (res?"succeeded":"failed");

    return res;
}

bool KSScreenManager::getActive()
{
    return m_active;
}

void KSScreenManager::setLockActive(bool lockActive)
{
    if (m_lockActive == lockActive)
    {
        QString debugTips = QString("trying to %1 locker,when locker is already %1").arg(lockActive?"activate":"inactivate");
        KLOG_DEBUG() << debugTips;
        return;
    }

    KLOG_DEBUG() << (lockActive?"activate":"inactivate") << "locker...";

    m_lockActive = lockActive;

    if (m_lockActive)
    {
        m_lockerDemo->setActive(true);
        m_screensaver->setActive(false);
    }
    else
    {
        m_lockerDemo->setActive(false);
        m_screensaver->setActive(true);
    }

    //TODO:暂时只在主屏上显示和模糊
    for (auto iter = m_windowMap.begin(); iter != m_windowMap.end(); iter++)
    {
        if (m_lockActive && iter.value() == m_lockerDemo->parentWidget())
        {
            iter.value()->startBlur();
        }
        else
        {
            iter.value()->resetBlur();
        }
    }
};

bool KSScreenManager::getLockActive() const
{
    return m_lockActive;
}

bool KSScreenManager::requestUnlock()
{
    //TODO:请求解锁，由屏保->解锁框
    return false;
}

void KSScreenManager::handleScreenAdded(QScreen *screen)
{
    if (!m_active)
    {
        return;
    }

    createWindowForScreen(screen);
    requestUnlock();
}

void KSScreenManager::handleScreenRemoved(QScreen *screen)
{
    if (!m_active)
    {
        return;
    }

    for (auto iter = m_windowMap.begin();
         iter != m_windowMap.end();
         iter++)
    {
        if (iter.key() == screen)
        {
            delete iter.value();
            m_windowMap.remove(iter.key());
            break;
        }
    }
}

void KSScreenManager::createWindowForScreen(QScreen *screen)
{
    auto window = new KSWindow(false, screen);
    KLOG_DEBUG() << "create window for screen:" << window->objectName();

    window->setScreen(screen);
    window->setBackground(m_background);

    window->show();

    m_windowMap.insert(screen, window);
}

bool KSScreenManager::activate()
{
    if (m_active)
    {
        KLOG_ERROR() << "trying to activate screen manager,when is already activate";
        return false;
    }

    /// 获取桌面壁纸路径
    QGSettings mateBackgroundSettings("org.mate.background");
    QVariant background = mateBackgroundSettings.get("picture-filename");
    if (!m_background.load(background.toString()))
    {
        KLOG_WARNING() << "can't load background," << background.toString();
    }

    /// 创建背景窗口覆盖所有的屏幕
    createWindows();

    /// 创建解锁框,屏保框
    auto findPrimaryWindowIter = m_windowMap.find(qApp->primaryScreen());
    KSWindow *defaultWindow = findPrimaryWindowIter == m_windowMap.end() ? nullptr : findPrimaryWindowIter.value();
    // TODO: 还未处理不存在屏幕的情况
    m_lockerDemo = new KSLockerDemo(m_enableAnimation, defaultWindow);
    m_screensaver = new KSScreensaver(m_enableAnimation, defaultWindow);
    m_lockerDemo->stackUnder(m_screensaver);
    m_screensaver->setVisible(true);
    m_lockerDemo->setVisible(true);

    connect(qApp, &QApplication::screenAdded, this, &KSScreenManager::handleScreenAdded);
    connect(qApp, &QApplication::screenRemoved, this, &KSScreenManager::handleScreenRemoved);

    /// 屏保激活,应延迟复位
    QTimer::singleShot(500, [this]() {
        m_fade->reset();
    });

    m_lockActive = false;
    m_active = true;
    return true;
}

void KSScreenManager::createWindows()
{
    auto screens = qApp->screens();

    for (auto &screen : screens)
    {
        createWindowForScreen(screen);
    }
}

bool KSScreenManager::deactivate()
{
    if (!m_active)
    {
        KLOG_ERROR() << "trying to deactivate screen manager,when is already deactivate";
        return false;
    }

    disconnect(qApp, &QApplication::screenAdded, this, &KSScreenManager::handleScreenAdded);
    disconnect(qApp, &QApplication::screenRemoved, this, &KSScreenManager::handleScreenRemoved);

    destroyWindows();

    delete m_screensaver;
    m_screensaver = nullptr;

    delete m_lockerDemo;
    m_lockerDemo = nullptr;

    m_lockActive = false;
    m_active = false;
    return true;
}

void KSScreenManager::destroyWindows()
{
    for (auto iter = m_windowMap.begin();
         iter != m_windowMap.end();)
    {
        delete iter.value();
        m_windowMap.erase(iter++);
    }
}

bool KSScreenManager::eventFilter(QObject *watched, QEvent *event)
{
    if (!m_active)
    {
        return false;
    }

    switch (event->type())
    {
    case QEvent::MouseButtonPress:
    {
        setLockActive(true);
        break;
    }
    case QEvent::KeyPress:
    {
        auto keyEvent = dynamic_cast<QKeyEvent *>(event);
        if (keyEvent->key() == Qt::Key_Escape)
        {
            setLockActive(false);
        }
        else
        {
            setLockActive(true);
        }
        break;
    }
    }

    return QObject::eventFilter(watched, event);
}
