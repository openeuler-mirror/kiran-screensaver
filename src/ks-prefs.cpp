//
// Created by lxh on 2021/7/2.
//

#include "ks-prefs.h"
#include <qt5-log-i.h>

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

KSPrefs::KSPrefs(QObject* parent)
    : QObject(parent)
{
}

KSPrefs::~KSPrefs()
{
}

bool KSPrefs::init()
{
    delete m_screensaverSettings;
    m_screensaverSettings = new QGSettings(SCHEMA_KIRAN_SCREENSAVER, "", this);
    if (!connect(m_screensaverSettings, &QGSettings::changed, this, &KSPrefs::handleGSettingsChanged))
    {
        KLOG_WARNING() << "can't connect screensaver settings changed!";
    }

    m_idleActivationLock = m_screensaverSettings->get(KEY_IDLE_ACTIVATION_LOCK).toBool();
    m_canLogout = m_screensaverSettings->get(KEY_CAN_LOGOUT).toBool();
    m_canUserSwitch = m_screensaverSettings->get(KEY_CAN_USER_SWITCH).toBool();

    ///输出设置项
    // clang-format off
    KLOG_DEBUG() << "load kiran-screensaver prefs:" << "\n"
                 << "\t" << KEY_CAN_LOGOUT << m_canLogout << "\n"
                 << "\t" << KEY_CAN_USER_SWITCH << m_canUserSwitch;
    // clang-format on

    return true;
}

bool KSPrefs::getIdleActivationLock() const
{
    return m_idleActivationLock;
}

bool KSPrefs::getCanLogout() const
{
    return m_canLogout;
}

bool KSPrefs::getCanUserSwitch() const
{
    return m_canUserSwitch;
}

void KSPrefs::setIdleActivationLock(bool idleActivationLock)
{
    RETURN_IF_SAME(m_idleActivationLock, idleActivationLock);
    m_idleActivationLock = idleActivationLock;
    m_screensaverSettings->set(KEY_IDLE_ACTIVATION_LOCK, m_idleActivationLock);
}

void KSPrefs::setCanLogout(bool canLogout)
{
    RETURN_IF_SAME(m_canLogout,canLogout);
    m_canLogout = canLogout;
    m_screensaverSettings->set(KEY_CAN_LOGOUT,canLogout);
}

void KSPrefs::setCanUserSwitch(bool canUserSwitch)
{
    RETURN_IF_SAME(m_canUserSwitch,canUserSwitch);
    m_canUserSwitch = canUserSwitch;
    m_screensaverSettings->set(KEY_CAN_USER_SWITCH,canUserSwitch);
}

void KSPrefs::handleGSettingsChanged(const QString& key)
{
    const QMap<QString, bool*> boolKeyMap = {
        {KEY_IDLE_ACTIVATION_LOCK, &m_idleActivationLock},
        {KEY_CAN_LOGOUT, &m_canLogout},
        {KEY_CAN_USER_SWITCH, &m_canUserSwitch}};

    auto boolIter = boolKeyMap.find(key);
    if (boolIter != boolKeyMap.end())
    {
        *boolIter.value() = m_screensaverSettings->get(boolIter.key()).toBool();
    }
}