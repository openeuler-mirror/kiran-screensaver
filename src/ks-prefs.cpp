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

#define SCHEMA_KIRAN_SCREENSAVER "com.kylinsec.kiran.screensaver"
#define KEY_IDLE_ACTIVATION_SCREENSAVER "idle-activation-screensaver"
#define KEY_SCREENSAVER_ACTIVATION_LOCK "screensaver-activation-lock"
#define KEY_CAN_LOGOUT "can-logout"
#define KEY_CAN_LOCK "can-lock"
#define KEY_CAN_USER_SWITCH "can-user-switch"
#define KEY_SCREENSAVER_MODE "screensaver-mode"
#define KEY_SCREENSAVER_THEME "screensaver-theme"

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

    m_idleActivationScreensaver = m_screensaverSettings->get(KEY_IDLE_ACTIVATION_SCREENSAVER).toBool();
    m_screensaverActivationLock = m_screensaverSettings->get(KEY_SCREENSAVER_ACTIVATION_LOCK).toBool();
    m_canLogout = m_screensaverSettings->get(KEY_CAN_LOGOUT).toBool();
    m_canLock = m_screensaverSettings->get(KEY_CAN_LOCK).toBool();
    m_canUserSwitch = m_screensaverSettings->get(KEY_CAN_USER_SWITCH).toBool();

    bool toUintOk = false;
    auto modeVar = m_screensaverSettings->get(KEY_SCREENSAVER_MODE);
    m_screensaverMode = modeVar.toUInt(&toUintOk);

    m_screenSaverTheme = m_screensaverSettings->get(KEY_SCREENSAVER_THEME).toString();

    ///输出设置项
    // clang-format off
    KLOG_DEBUG() << "load kiran-screensaver prefs:" << "\n"
                 << "\t" << KEY_IDLE_ACTIVATION_SCREENSAVER << m_idleActivationScreensaver << "\n"
                 << "\t" << KEY_SCREENSAVER_ACTIVATION_LOCK << m_screensaverActivationLock << "\n"
                 << "\t" << KEY_CAN_LOGOUT << m_canLogout << "\n"
                 << "\t" << KEY_CAN_LOCK << m_canLock << "\n"
                 << "\t" << KEY_CAN_USER_SWITCH << m_canUserSwitch << "\n"
                 << "\t" << KEY_SCREENSAVER_MODE << m_screensaverMode << "\n"
                 << "\t" << KEY_SCREENSAVER_THEME << m_screenSaverTheme;
    // clang-format on

    return true;
}

bool KSPrefs::getIdleActivationScreensaver()
{
    return m_idleActivationScreensaver;
}

bool KSPrefs::getScreensaverActivationLock()
{
    return m_screensaverActivationLock;
}

bool KSPrefs::getCanLogout()
{
    return m_canLogout;
}

bool KSPrefs::getCanLock()
{
    return m_canLock;
}

bool KSPrefs::getCanUserSwitch()
{
    return m_canUserSwitch;
}

uint KSPrefs::getScreenSaverMode()
{
    return m_screensaverMode;
}

QString KSPrefs::getScreenSaverTheme()
{
    return m_screenSaverTheme;
}

void KSPrefs::setIdleActivationScreensaver(bool idleActivationScreensaver)
{
    RETURN_IF_SAME(m_idleActivationScreensaver, idleActivationScreensaver);
    m_idleActivationScreensaver = idleActivationScreensaver;
    m_screensaverSettings->set(KEY_IDLE_ACTIVATION_SCREENSAVER, idleActivationScreensaver);
}

void KSPrefs::setScreensaverActivationLock(bool screensaverActivationLock)
{
    RETURN_IF_SAME(m_screensaverActivationLock, screensaverActivationLock);
    m_screensaverActivationLock = screensaverActivationLock;
    m_screensaverSettings->set(KEY_SCREENSAVER_ACTIVATION_LOCK, screensaverActivationLock);
}

void KSPrefs::setCanLogout(bool canLogout)
{
    RETURN_IF_SAME(m_canLogout,canLogout);
    m_canLogout = canLogout;
    m_screensaverSettings->set(KEY_CAN_LOGOUT,canLogout);
}

void KSPrefs::setCanLock(bool canLock)
{
    RETURN_IF_SAME(m_canLock,canLock);
    m_canLock = canLock;
    m_screensaverSettings->set(KEY_CAN_LOCK,canLock);
}

void KSPrefs::setCanUserSwitch(bool canUserSwitch)
{
    RETURN_IF_SAME(m_canUserSwitch,canUserSwitch);
    m_canUserSwitch = canUserSwitch;
    m_screensaverSettings->set(KEY_CAN_USER_SWITCH,canUserSwitch);
}

void KSPrefs::setScreenSaverMode(uint mode)
{
    RETURN_IF_SAME(m_screensaverMode,mode);
    m_screensaverMode = mode;
    m_screensaverSettings->set(KEY_SCREENSAVER_MODE,mode);
}

void KSPrefs::setScreenSaverTheme(QString theme)
{
    RETURN_IF_SAME(m_screenSaverTheme,theme);
    m_screenSaverTheme = theme;
    m_screensaverSettings->set(KEY_SCREENSAVER_THEME,theme);
}

void KSPrefs::handleGSettingsChanged(const QString& key)
{
    const QMap<QString, bool*> boolKeyMap = {
        {KEY_IDLE_ACTIVATION_SCREENSAVER, &m_idleActivationScreensaver},
        {KEY_SCREENSAVER_ACTIVATION_LOCK, &m_screensaverActivationLock},
        {KEY_CAN_LOGOUT, &m_canLogout},
        {KEY_CAN_LOCK, &m_canLock},
        {KEY_CAN_USER_SWITCH, &m_canUserSwitch}};

    auto boolIter = boolKeyMap.find(key);
    if (boolIter != boolKeyMap.end())
    {
        *boolIter.value() = m_screensaverSettings->get(boolIter.key()).toBool();
    }
    else if (key == KEY_SCREENSAVER_MODE)
    {
        bool isOk = false;
        m_screensaverMode = m_screensaverSettings->get(KEY_SCREENSAVER_MODE).toUInt(&isOk);
        if (!isOk || (m_screensaverMode < 0 || m_screensaverMode > MODE_LAST))
        {
            KLOG_WARNING() << "key:" << KEY_SCREENSAVER_MODE << "is not valid";
            m_screensaverMode = MODE_BUILD_IN;
        }
    }
    else if (key == KEY_SCREENSAVER_THEME)
    {
        m_screenSaverTheme = m_screensaverSettings->get(KEY_SCREENSAVER_THEME).toString();
    }
}