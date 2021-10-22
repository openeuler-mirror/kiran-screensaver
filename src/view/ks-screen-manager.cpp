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
#include "ks-screen-manager.h"
#include "ks-fade.h"
#include "ks-grab.h"
#include "ks-locker-interface.h"
#include "ks-plugin-interface.h"
#include "ks-prefs.h"
#include "ks-window.h"
#include "screensaver/ks-screensaver.h"
#include "ks-animation-define.h"

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
    //加载解锁框插件
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
        //传入kiran-screensaver提供相关接口进行插件初始化
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
            m_lockerInterface->setAnimationDelay(UNLOCK_DIALOG_FADE_IN_ANIMATION_DELAY_MS,UNLOCK_DIALOG_FADE_OUT_ANIMATION_DELAY_MS);
            m_lockerInterface->setAnimationDuration(UNLOCK_DIALOG_FADE_IN_ANIMATION_DURATION_MS, UNLOCK_DIALOG_FADE_OUT_ANIMATION_DURATION_MS);
            m_lockerInterface->setEnableSwitch(m_prefs->getCanUserSwitch());
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
        m_lockerInterface->fadeIn();
        m_screensaver->setMaskState(false);
    }
    else
    {
        m_lockerInterface->fadeOut();
        m_screensaver->setMaskState(true);
    }

    setBackgroundWindowBlured(m_currentWindow);
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

    deleteWindowForScreen(screen);
}

KSWindow *KSScreenManager::createWindowForScreen(QScreen *screen)
{
    auto window = new KSWindow(m_enableAnimation, screen);
    connect(window,&KSWindow::mouseEnter,this,&KSScreenManager::handleWindowMouseEnter);

    KLOG_DEBUG() << "create window for screen:" << window->objectName();

    window->setScreen(screen);
    window->setBackground(m_background);

    window->show();

    m_windowMap.insert(screen, window);

    return window;
}

void KSScreenManager::deleteWindowForScreen(QScreen *screen)
{
    auto iter = m_windowMap.find(screen);
    if (iter != m_windowMap.end())
    {
        auto screenBackground = iter.value();
        m_windowMap.erase(iter);

        // 若是当前内容窗口所在的屏幕被移除时，将内容窗口移动到另一个屏幕上
        if (m_currentWindow == screenBackground)
        {
            KSWindow* newScreenBackground = nullptr;
            if( m_windowMap.size() > 0 )
            {
                newScreenBackground = m_windowMap.values().at(0);
            }
            moveContentToWindow(newScreenBackground);
            m_currentWindow = newScreenBackground;
        }

        delete iter.value();
        m_windowMap.erase(iter);
    }
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
    ///过滤全局事件，判断是否激活解锁框，或启用屏保盖住解锁框
    if (eventFilterActivate(watched, event))
    {
        return true;
    }

    /// 过滤全局时间，判断窗口Resize事件，调整窗口大小
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
    m_currentWindow = window;

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

    if(m_lockerVisible)
    {
        setBackgroundWindowBlured(m_currentWindow);
    }

    if (m_currentWindow)
    {
        if( !KSGrab::getInstance()->grabWindow(m_currentWindow->winId(), false) )
        {
            KLOG_WARNING() << "grab to current window:" << m_currentWindow->objectName() << "failed!";
        }
        else
        {
            KLOG_DEBUG() << "grab to current window:" << m_currentWindow->objectName() << "failed!";
        }
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

    if (m_lockerInterface != nullptr)
    {
        m_lockerInterface->get_widget_ptr()->resize(size);
    }

    if (m_screensaver != nullptr)
    {
        m_screensaver->resize(size);
    }
}

void KSScreenManager::handleWindowMouseEnter()
{
    KSWindow* window = qobject_cast<KSWindow*>(sender());
    moveContentToWindow(window);
}

void KSScreenManager::setBackgroundWindowBlured(KSWindow *window)
{
    // 设置背景窗口模糊
    for (auto iter = m_windowMap.begin(); iter != m_windowMap.end(); iter++)
    {
        if (m_lockerVisible && iter.value() == window)
        {
            iter.value()->setBlurBackground(true);
        }
        else
        {
            iter.value()->setBlurBackground(false);
        }
    }
}
