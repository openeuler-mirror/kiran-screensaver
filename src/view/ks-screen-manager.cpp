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
#include "ks-locker-interface.h"
#include "ks-plugin-interface.h"
#include "ks-prefs.h"
#include "ks-window.h"
#include "screensaver/ks-screensaver.h"

#include <qt5-log-i.h>
#include <QApplication>
#include <QGSettings>
#include <QKeyEvent>
#include <QPluginLoader>
#include <QScreen>
#include <QStateMachine>
#include <QTimer>

KSScreenManager::KSScreenManager(KSFade *fade,
                                 QObject *parent)
    : QObject(parent),
      m_fade(fade),
      m_prefs(KSPrefs::getInstance())
{
    if (m_prefs != nullptr)
    {
        m_enableAnimation = m_prefs->getEnableAnimation();
        m_idleActivationLock = m_prefs->getIdleActivationLock();
    }

    QCoreApplication::instance()->installEventFilter(this);
}

KSScreenManager::~KSScreenManager()
{
    destroyWindows();
}

bool KSScreenManager::init()
{
    //加载相关插件
    QString lockerPluginPath = KSPrefs::getInstance()->getLockerPluginPath();
    QPluginLoader pluginLoader(lockerPluginPath);
    if (!pluginLoader.load())
    {
        KLOG_ERROR() << "can't load plugin:" << pluginLoader.errorString();
        m_lockerPluginInterface = nullptr;
    }
    else
    {
        KLOG_DEBUG() << "load" << pluginLoader.fileName() << "succeed!";
        KSPluginInterface *pluginInterface = qobject_cast<KSPluginInterface *>(pluginLoader.instance());
        int iRes = pluginInterface->init(this);
        if (iRes != 0)
        {
            KLOG_ERROR() << "init kiran screensaver plugin failed!" << iRes;
            pluginLoader.unload();
        }
        else
        {
            m_lockerPluginInterface = qobject_cast<KSPluginInterface *>(pluginLoader.instance());
        }
    }

    return true;
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

bool KSScreenManager::setLockActive(bool lockActive)
{
    if (m_lockActive == lockActive)
    {
        QString debugTips = QString("trying to %1 locker,when locker is already %1").arg(lockActive ? "activate" : "inactivate");
        KLOG_DEBUG() << debugTips;
        return true;
    }

    KLOG_DEBUG() << (lockActive ? "activate" : "inactivate") << "locker...";

    if (lockActive)
    {
        if (!m_lockerPluginInterface)
        {
            KLOG_DEBUG() << "activate locker failed,can't load locker plugin!";
            return false;
        }
        else
        {
            m_lockerInterface = m_lockerPluginInterface->createLocker();
            m_lockerInterface->setAnimationEnabled(m_enableAnimation);
            m_lockerInterface->setAnimationDuration(1500, 50);
            moveContentToWindow(m_currentWindow);
        }
    }
    else
    {
        delete m_lockerInterface;
        m_lockerInterface = nullptr;
        m_lockerVisible = false;
    }

    m_lockActive = lockActive;

    return true;
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
        m_lockerInterface->fadeIn();
    }
    else
    {
        m_screensaver->setMaskState(true);
        m_lockerInterface->fadeOut();
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

bool KSScreenManager::getLockVisible()
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

    // 创建解锁框,屏保框
    m_screensaver = new KSScreensaver(m_enableAnimation, nullptr);

    if (m_idleActivationLock)
    {
        if (!setLockActive(true))
        {
            delete m_screensaver;
            return false;
        }
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

    auto findPrimaryWindowIter = m_windowMap.find(qApp->primaryScreen());
    KSWindow *defaultWindow = findPrimaryWindowIter == m_windowMap.end() ? nullptr : findPrimaryWindowIter.value();
    moveContentToWindow(defaultWindow);

    connect(qApp, &QApplication::screenAdded, this, &KSScreenManager::handleScreenAdded);
    connect(qApp, &QApplication::screenRemoved, this, &KSScreenManager::handleScreenRemoved);

    // 屏保激活,应延迟复位
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
    if (eventFilterActivate(watched, event))
    {
        return true;
    }
    if (eventFilterCurrentWindowResize(watched, event))
    {
        return true;
    }
    return QObject::eventFilter(watched, event);
}

bool KSScreenManager::eventFilterActivate(QObject *watched, QEvent *event)
{
    if (!m_active)
        return false;

    // 若解锁框已被激活　按键和鼠标事件触发解锁框显示
    // 若解锁款未被激活　按键和鼠标时间应该设置KSListener　deactivate,取消掉屏保
    if (event->type() == QEvent::MouseButtonPress)
    {
        if (getLockActive() && !getLockVisible())
        {
            setLockVisible(true);
            return true;
        }
        else if (!getLockActive())
        {
            emit sigReqDeactivated();
        }
    }
    else if (event->type() == QEvent::KeyPress)
    {
        auto keyEvent = dynamic_cast<QKeyEvent *>(event);
        if (!getLockActive())
        {
            emit sigReqDeactivated();
        }
        else
        {
            if (getLockActive() && getLockVisible() && keyEvent->key() == Qt::Key_Escape)
            {
                setLockVisible(false);
                return true;
            }
            else if (getLockActive() && !getLockVisible())
            {
                setLockVisible(true);
                return true;
            }
        }
    }

    return false;
}

bool KSScreenManager::eventFilterCurrentWindowResize(QObject *watched, QEvent *event)
{
    if ((watched != nullptr) && (watched == m_currentWindow) && (event->type() == QEvent::Resize))
    {
        auto resizeEvent = dynamic_cast<QResizeEvent *>(event);
        updateCurrentSubWindowGeometry(resizeEvent->size());
    }

    return false;
}

void KSScreenManager::moveContentToWindow(KSWindow *window)
{
    if (m_screensaver != nullptr)
    {
        m_screensaver->setParent(window);
        m_screensaver->setVisible(window != nullptr);
    }

    if (m_lockerInterface != nullptr)
    {
        m_lockerInterface->get_widget_ptr()->setParent(window);
        m_lockerInterface->get_widget_ptr()->setVisible(window != nullptr);
        m_lockerInterface->get_widget_ptr()->stackUnder(m_screensaver);
    }

    m_currentWindow = window;

    if (m_currentWindow)
    {
        KLOG_DEBUG() << "grab to current window:" << m_currentWindow->objectName();
        KSGrab::getInstance()->grabWindow(m_currentWindow->winId(), false);
        updateCurrentSubWindowGeometry(window->size());
    }
    else
    {
        KSGrab::getInstance()->grabRoot(false);
    }
}

void KSScreenManager::authenticationPassed()
{
    emit sigReqDeactivated();
}

void KSScreenManager::updateCurrentSubWindowGeometry(QSize size)
{
    QRect rect(0, 0, size.width(), size.height());

    if (m_lockerInterface != nullptr)
    {
        m_lockerInterface->get_widget_ptr()->setGeometry(rect);
    }

    if (m_screensaver != nullptr)
    {
        m_screensaver->setGeometry(rect);
    }
}
