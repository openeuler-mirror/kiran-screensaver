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

#define SCHEMA_KIRAN_SCREENSAVER        "com.kylinsec.kiran.screensaver"
#define KEY_IDLE_ACTIVATION_LOCK        "idle-activation-lock"
#define KEY_CAN_LOGOUT                  "can-logout"
#define KEY_CAN_USER_SWITCH             "can-user-switch"
#define KEY_ENABLE_ANIMATION            "enable-animation"
#define KEY_SCREENSAVER_LOCKER          "screensaver-locker"

using namespace Kiran::ScreenSaver;

Prefs::Prefs(QObject* parent)
    : QObject(parent)
{
}

Prefs::~Prefs()
{
}

bool Prefs::init()
{
    if(isInited)
    {
        return true;
    }

    delete m_screensaverSettings;
    m_screensaverSettings = new QGSettings(SCHEMA_KIRAN_SCREENSAVER, "", this);
    if (!connect(m_screensaverSettings, &QGSettings::changed, this, &Prefs::handleGSettingsChanged))
    {
        KLOG_WARNING() << "can't connect screensaver settings changed!";
    }

    //m_idleActivationLock = m_screensaverSettings->get(KEY_IDLE_ACTIVATION_LOCK).toBool();
    m_canLogout = m_screensaverSettings->get(KEY_CAN_LOGOUT).toBool();
    m_canUserSwitch = m_screensaverSettings->get(KEY_CAN_USER_SWITCH).toBool();
    m_enableAnimation = m_screensaverSettings->get(KEY_ENABLE_ANIMATION).toBool();
    m_lockerPluginPath = m_screensaverSettings->get(KEY_SCREENSAVER_LOCKER).toString();

    ///输出设置项
    KLOG_DEBUG() << "load kiran-screensaver prefs:";
    //KLOG_DEBUG() << "\t" KEY_IDLE_ACTIVATION_LOCK << m_idleActivationLock;
    KLOG_DEBUG() << "\t" KEY_CAN_LOGOUT << m_canLogout;
    KLOG_DEBUG() << "\t" KEY_CAN_USER_SWITCH << m_canUserSwitch;
    KLOG_DEBUG() << "\t" KEY_ENABLE_ANIMATION << m_enableAnimation;
    KLOG_DEBUG() << "\t" KEY_SCREENSAVER_LOCKER << m_lockerPluginPath;

    isInited = true;
    return true;
}

bool Prefs::getIdleActivationLock() const
{
    return m_idleActivationLock;
}

bool Prefs::getCanLogout() const
{
    return m_canLogout;
}

bool Prefs::getCanUserSwitch() const
{
    return m_canUserSwitch;
}

/*
void Prefs::setIdleActivationLock(bool idleActivationLock)
{
    RETURN_IF_SAME(m_idleActivationLock, idleActivationLock);
    m_idleActivationLock = idleActivationLock;
    m_screensaverSettings->set(KEY_IDLE_ACTIVATION_LOCK, m_idleActivationLock);
}
*/

void Prefs::setCanLogout(bool canLogout)
{
    RETURN_IF_SAME(m_canLogout,canLogout);
    m_canLogout = canLogout;
    m_screensaverSettings->set(KEY_CAN_LOGOUT,canLogout);
}

void Prefs::setCanUserSwitch(bool canUserSwitch)
{
    RETURN_IF_SAME(m_canUserSwitch,canUserSwitch);
    m_canUserSwitch = canUserSwitch;
    m_screensaverSettings->set(KEY_CAN_USER_SWITCH,canUserSwitch);
}

void Prefs::handleGSettingsChanged(const QString& key)
{
    const QMap<QString, bool*> boolKeyMap = {
        //{KEY_IDLE_ACTIVATION_LOCK, &m_idleActivationLock},
        {KEY_CAN_LOGOUT, &m_canLogout},
        {KEY_CAN_USER_SWITCH, &m_canUserSwitch},
        {KEY_ENABLE_ANIMATION,&m_enableAnimation}
    };

    auto boolIter = boolKeyMap.find(key);
    if (boolIter != boolKeyMap.end())
    {
        *boolIter.value() = m_screensaverSettings->get(boolIter.key()).toBool();
    }
}

bool Prefs::getEnableAnimation() const
{
    return m_enableAnimation;
}

void Prefs::setEnableAnimation(bool enableAnimation)
{
    RETURN_IF_SAME(m_enableAnimation,enableAnimation);
    m_enableAnimation = enableAnimation;
    m_screensaverSettings->set(KEY_ENABLE_ANIMATION,m_enableAnimation);
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
