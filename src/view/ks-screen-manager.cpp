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
#include "ks-grab.h"
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
        m_idleActivationLock = m_prefs->getIdleActivationLock();
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

    KLOG_DEBUG() << "set screen manager:" << (active ? "activate" : "inactivate") << ":" << (res ? "succeeded" : "failed");

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
        QString debugTips = QString("trying to %1 locker,when locker is already %1").arg(lockActive ? "activate" : "inactivate");
        KLOG_DEBUG() << debugTips;
        return;
    }

    KLOG_DEBUG() << (lockActive ? "activate" : "inactivate") << "locker...";

    m_lockActive = lockActive;

    if (m_lockActive)
    {
        m_lockerDemo = new KSLockerDemo(m_enableAnimation);
        connect(m_lockerDemo, &KSLockerDemo::authenticationPassed, this, &KSScreenManager::sigReqDeactivated);
        moveContentToWindow(m_currentWindow);
    }
    else
    {
        delete m_lockerDemo;
        m_lockerDemo = nullptr;
        m_lockerVisible = false;
    }
};

bool KSScreenManager::getLockActive() const
{
    return m_lockActive;
}

void KSScreenManager::setLockVisible(bool lockVisible)
{
    if (m_lockerVisible == lockVisible)
    {
        return;
    }

    if (!m_lockActive && lockVisible)
    {
        // 锁定框还未激活,不能显示
        return;
    }

    m_lockerVisible = lockVisible;

    if (m_lockerVisible)
    {
        m_screensaver->setMaskState(false);
        m_lockerDemo->fadeIn();
    }
    else
    {
        m_screensaver->setMaskState(true);
        m_lockerDemo->fadeOut();
    }

    // 设置背景窗口模糊
    for (auto iter = m_windowMap.begin(); iter != m_windowMap.end(); iter++)
    {
        if (m_lockerVisible && iter.value() == m_currentWindow)
        {
            iter.value()->setBlurBackground(true);
        }
        else
        {
            iter.value()->setBlurBackground(false);
        }
    }
}

bool KSScreenManager::getLockVisible(bool lockVisible)
{
    return m_lockerVisible;
}

void KSScreenManager::handleScreenAdded(QScreen *screen)
{
    if (!m_active)
    {
        return;
    }

    KSWindow *newWindow = createWindowForScreen(screen);
    if (m_currentWindow == nullptr)
    {
        moveContentToWindow(newWindow);
    }
}

void KSScreenManager::handleScreenRemoved(QScreen *screen)
{
    if (!m_active)
    {
        return;
    }

    auto iter = m_windowMap.find(screen);
    if (iter != m_windowMap.end())
    {
        if (m_currentWindow == iter.value())
        {
            moveContentToWindow(nullptr);
            m_currentWindow = nullptr;
        }
        delete iter.value();
        m_windowMap.erase(iter);
    }
}

KSWindow *KSScreenManager::createWindowForScreen(QScreen *screen)
{
    auto window = new KSWindow(m_enableAnimation, screen);
    KLOG_DEBUG() << "create window for screen:" << window->objectName();

    window->setScreen(screen);
    window->setBackground(m_background);

    window->show();

    m_windowMap.insert(screen, window);

    return window;
}

bool KSScreenManager::activate()
{
    if (m_active)
    {
        KLOG_ERROR() << "trying to activate screen manager,when is already activate";
        return false;
    }

    // 获取桌面壁纸路径
    QGSettings mateBackgroundSettings("org.mate.background");
    QVariant background = mateBackgroundSettings.get("picture-filename");
    if (!m_background.load(background.toString()))
    {
        KLOG_WARNING() << "can't load background," << background.toString();
    }

    // 创建背景窗口覆盖所有的屏幕
    createWindows();

    // 创建解锁框,屏保框
    m_screensaver = new KSScreensaver(m_enableAnimation, nullptr);
    // 若开启空闲时激活锁定屏幕,才在激活时创建锁定框
    if (m_idleActivationLock)
    {
        setLockActive(true);
    }
    auto findPrimaryWindowIter = m_windowMap.find(qApp->primaryScreen());
    KSWindow *defaultWindow = findPrimaryWindowIter == m_windowMap.end() ? nullptr : findPrimaryWindowIter.value();
    moveContentToWindow(defaultWindow);

    connect(qApp, &QApplication::screenAdded, this, &KSScreenManager::handleScreenAdded);
    connect(qApp, &QApplication::screenRemoved, this, &KSScreenManager::handleScreenRemoved);

    /// 屏保激活,应延迟复位
    QTimer::singleShot(500, [this]() {
        m_fade->reset();
    });

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

    delete m_screensaver;
    m_screensaver = nullptr;

    setLockActive(false);

    KSGrab::getInstance()->releaseGrab();

    destroyWindows();

    m_currentWindow = nullptr;
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
        if (getLockActive())
        {
            setLockVisible(true);
        }
        else
        {
            emit sigReqDeactivated();
        }
        break;
    }
    case QEvent::KeyPress:
    {
        auto keyEvent = dynamic_cast<QKeyEvent *>(event);
        if( !getLockActive() )
        {
            emit sigReqDeactivated();
        }
        else
        {
            if (keyEvent->key() == Qt::Key_Escape)
            {
                setLockVisible(false);
            }
            else
            {
                setLockVisible(true);
            }
        }
    }
    break;
    }

    return QObject::eventFilter(watched, event);
}

void KSScreenManager::moveContentToWindow(KSWindow *window)
{
    if (m_screensaver != nullptr)
    {
        m_screensaver->setParent(window);
        m_screensaver->setVisible(window != nullptr);
    }

    if (m_lockerDemo != nullptr)
    {
        m_lockerDemo->setParent(window);
        m_lockerDemo->setVisible(window != nullptr);
        m_lockerDemo->stackUnder(m_screensaver);
    }

    m_currentWindow = window;

    if (m_currentWindow)
    {
        KLOG_DEBUG() << "grab to current window:" << m_currentWindow->objectName();
        KSGrab::getInstance()->grabWindow(m_currentWindow->winId(),false);
    }
    else
    {
        KSGrab::getInstance()->grabRoot(false);
    }
}