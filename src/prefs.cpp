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
#include "prefs.h"
#include <qt5-log-i.h>
#include <QGSettings>
#include <QMutex>
#include <QScopedPointer>
#define RETURN_IF_SAME(value_1, value_2)  \
    {                                     \
        if (value_1 == value_2)           \
        {                                 \
            KLOG_DEBUG("is same,ignore"); \
            return;                       \
        }                                 \
    }

#define SCHEMA_KIRAN_SCREENSAVER "com.kylinsec.kiran.screensaver"

/**
 * KEY_SPLIT_SCREENSAVER_AND_LOCK=false,KEY_IDLE_ACTIVATION_LOCK标志空闲时触发锁屏以及屏保
 * KEY_SPLIT_SCREENSAVER_AND_LOCK=true, KEY_IDLE_ACTIVATION_LOCK标志空闲时触发锁屏 KEY_IDLE_ACTIVATION_SCREENSAVER标志空闲时触发屏保
 */
#define KEY_SPLIT_SCREENSAVER_AND_LOCK "splitScreensaverAndLock"
#define KEY_IDLE_ACTIVATION_LOCK "idleActivationLock"
#define KEY_IDLE_ACTIVATION_SCREENSAVER "idleActivationScreensaver"

#define KEY_CAN_LOGOUT "canLogout"
#define KEY_CAN_USER_SWITCH "canUserSwitch"
#define KEY_ENABLE_ANIMATION "enableAnimation"
#define KEY_SCREENSAVER_LOCKER "screensaverLocker"
#define KEY_SCREENSAVER_THEME "screensaverTheme"

using namespace Kiran::ScreenSaver;

Prefs::Prefs(QObject* parent)
    : QObject(parent)
{
}

Prefs::~Prefs()
{
    KLOG_DEBUG() << "~Prefs";
}

bool Prefs::init()
{
    if (isInited)
    {
        return true;
    }

    delete m_screensaverSettings;
    m_screensaverSettings = new QGSettings(SCHEMA_KIRAN_SCREENSAVER, "", this);
    if (!connect(m_screensaverSettings, &QGSettings::changed, this, &Prefs::handleGSettingsChanged))
    {
        KLOG_WARNING() << "can't connect screensaver settings changed!";
    }

    KLOG_INFO() << "load kiran-screensaver prefs:";

    m_splitScreensaverAndLock = m_screensaverSettings->get(KEY_SPLIT_SCREENSAVER_AND_LOCK).toBool();
    KLOG_INFO() << "\t" KEY_SPLIT_SCREENSAVER_AND_LOCK << m_splitScreensaverAndLock;

    m_idleActivationLock = m_screensaverSettings->get(KEY_IDLE_ACTIVATION_LOCK).toBool();
    KLOG_INFO() << "\t" KEY_IDLE_ACTIVATION_LOCK << m_idleActivationLock;

    m_idleActivationScreensaver = m_screensaverSettings->get(KEY_IDLE_ACTIVATION_SCREENSAVER).toBool();
    KLOG_INFO() << "\t" KEY_IDLE_ACTIVATION_SCREENSAVER << m_idleActivationScreensaver;

    m_canLogout = m_screensaverSettings->get(KEY_CAN_LOGOUT).toBool();
    KLOG_INFO() << "\t" KEY_CAN_LOGOUT << m_canLogout;

    m_canUserSwitch = m_screensaverSettings->get(KEY_CAN_USER_SWITCH).toBool();
    KLOG_INFO() << "\t" KEY_CAN_USER_SWITCH << m_canUserSwitch;

    m_enableAnimation = m_screensaverSettings->get(KEY_ENABLE_ANIMATION).toBool();
    KLOG_INFO() << "\t" KEY_ENABLE_ANIMATION << m_enableAnimation;

    m_lockerPluginPath = m_screensaverSettings->get(KEY_SCREENSAVER_LOCKER).toString();
    KLOG_INFO() << "\t" KEY_SCREENSAVER_LOCKER << m_lockerPluginPath;

    m_screensaverTheme = m_screensaverSettings->get(KEY_SCREENSAVER_THEME).toString();
    KLOG_INFO() << "\t" KEY_SCREENSAVER_THEME << m_screensaverTheme;

    isInited = true;
    return true;
}

bool Prefs::getIdleActivationLock() const
{
    return m_idleActivationLock;
}

bool Kiran::ScreenSaver::Prefs::getIdleActivationScreensaver() const
{
    if( m_splitScreensaverAndLock )
    {
        return m_idleActivationScreensaver;
    }
    else
    {
        return m_idleActivationLock;
    }
}

bool Prefs::getCanLogout() const
{
    return m_canLogout;
}

bool Prefs::getCanUserSwitch() const
{
    return m_canUserSwitch;
}

void Prefs::setIdleActivationLock(bool idleActivationLock)
{
    RETURN_IF_SAME(m_idleActivationLock, idleActivationLock);
    m_idleActivationLock = idleActivationLock;
    m_screensaverSettings->set(KEY_IDLE_ACTIVATION_LOCK, m_idleActivationLock);
}

void Prefs::setCanLogout(bool canLogout)
{
    RETURN_IF_SAME(m_canLogout, canLogout);
    m_canLogout = canLogout;
    m_screensaverSettings->set(KEY_CAN_LOGOUT, canLogout);
}

void Prefs::setCanUserSwitch(bool canUserSwitch)
{
    RETURN_IF_SAME(m_canUserSwitch, canUserSwitch);
    m_canUserSwitch = canUserSwitch;
    m_screensaverSettings->set(KEY_CAN_USER_SWITCH, canUserSwitch);
}

void Prefs::handleGSettingsChanged(const QString& key)
{
    const QMap<QString, bool*> boolKeyMap = {
        {KEY_SPLIT_SCREENSAVER_AND_LOCK, &m_splitScreensaverAndLock},
        {KEY_IDLE_ACTIVATION_LOCK, &m_idleActivationLock},
        {KEY_IDLE_ACTIVATION_SCREENSAVER, &m_idleActivationScreensaver},
        {KEY_CAN_LOGOUT, &m_canLogout},
        {KEY_CAN_USER_SWITCH, &m_canUserSwitch},
        {KEY_ENABLE_ANIMATION, &m_enableAnimation}};

    auto boolIter = boolKeyMap.find(key);
    if (boolIter != boolKeyMap.end())
    {
        *boolIter.value() = m_screensaverSettings->get(boolIter.key()).toBool();
        KLOG_INFO() << "settings changed:" << key << *boolIter.value();
    }

    if (key == KEY_IDLE_ACTIVATION_LOCK)
    {
        emit idleActivationLockChanged();
    }
    else if (key == KEY_IDLE_ACTIVATION_SCREENSAVER && m_splitScreensaverAndLock)
    {
        emit idleActivationScrensaverChanged();
    }
    else if(key == KEY_SPLIT_SCREENSAVER_AND_LOCK)
    {
        emit idleActivationScrensaverChanged();
    }
    else if (key == KEY_SCREENSAVER_THEME)
    {
        m_screensaverTheme = m_screensaverSettings->get(key).toString();
    }
}

bool Prefs::getEnableAnimation() const
{
    return m_enableAnimation;
}

QString Prefs::getScreensaverTheme() const
{
    return m_screensaverTheme;
}

void Prefs::setEnableAnimation(bool enableAnimation)
{
    RETURN_IF_SAME(m_enableAnimation, enableAnimation);
    m_enableAnimation = enableAnimation;
    m_screensaverSettings->set(KEY_ENABLE_ANIMATION, m_enableAnimation);
}

Prefs* Prefs::getInstance()
{
    static QMutex mutex;
    static QScopedPointer<Prefs> pInst;

    if (Q_UNLIKELY(!pInst))
    {
        QMutexLocker locker(&mutex);
        if (pInst.isNull())
        {
            pInst.reset(new Prefs());
        }
    }

    return pInst.data();
}

QString Prefs::getLockerPluginPath() const
{
    return m_lockerPluginPath;
}
