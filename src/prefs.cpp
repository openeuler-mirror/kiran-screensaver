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
#include "config.h"
#include <QFile>
#include <QTimer>

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

#define SCHEMA_KIRAN_POWER "com.kylinsec.kiran.power"
#define KEY_ENABLE_DISPLAY_IDLE_DIMMED "enableDisplayIdleDimmed"

#define SCHEMA_KIRAN_APPEARANCE "com.kylinsec.kiran.appearance"
#define KEY_LOCK_SCREEN_BACKGROUND "lockScreenBackground"
#define DEFAULT_LOCK_SCREEN_BACKGROUND "/usr/share/backgrounds/kiran/default.jpg"

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

    // 尝试GSettings初始化
    if (QGSettings::isSchemaInstalled(SCHEMA_KIRAN_SCREENSAVER))
    {
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
    }

    if (QGSettings::isSchemaInstalled(SCHEMA_KIRAN_POWER))
    {
        m_powerSettings = new QGSettings(SCHEMA_KIRAN_POWER, "", this);
        if (!m_powerSettings->keys().contains(KEY_ENABLE_DISPLAY_IDLE_DIMMED))
        {
            delete m_powerSettings;
            m_powerSettings = nullptr;
        }
        else
        {
            m_enableDisplayIdleDimmed = m_powerSettings->get(KEY_ENABLE_DISPLAY_IDLE_DIMMED).toBool();
            connect(m_powerSettings, &QGSettings::changed, this, &Prefs::handlePowerGSettingsChanged);
            KLOG_INFO() << "load kiran-power prefs: enable-display-idle-dimmed" << m_enableDisplayIdleDimmed;
        }
    }

    if (QGSettings::isSchemaInstalled(SCHEMA_KIRAN_APPEARANCE))
    {
        m_appearanceSettings = new QGSettings(SCHEMA_KIRAN_APPEARANCE, "", this);
        connect(m_appearanceSettings, &QGSettings::changed, this, &Prefs::handleAppearanceGSettingsChanged);
        m_lockScreenBackground = m_appearanceSettings->get(KEY_LOCK_SCREEN_BACKGROUND).toString();
        KLOG_INFO() << "load kiran-appearance prefs: lock-screen-background" << m_lockScreenBackground;
    }

    // GSettings不可用时，用ini兜底
    if (!m_screensaverSettings || !m_powerSettings || !m_appearanceSettings)
    {
        m_screensaverSettingsIni = new QSettings(SCREENSAVER_CONFIG_FILE, QSettings::IniFormat, this);
        KLOG_INFO() << "using ini fallback:" << SCREENSAVER_CONFIG_FILE;

        fillMissingFromIni();
        setupIniFileWatcher();
    }

    isInited = true;
    return true;
}

void Prefs::fillMissingFromIni()
{
    if (!m_screensaverSettings)
    {
        m_splitScreensaverAndLock = m_screensaverSettingsIni->value(KEY_SPLIT_SCREENSAVER_AND_LOCK, false).toBool();
        m_idleActivationLock = m_screensaverSettingsIni->value(KEY_IDLE_ACTIVATION_LOCK, true).toBool();
        m_idleActivationScreensaver = m_screensaverSettingsIni->value(KEY_IDLE_ACTIVATION_SCREENSAVER, true).toBool();
        m_canLogout = m_screensaverSettingsIni->value(KEY_CAN_LOGOUT, true).toBool();
        m_canUserSwitch = m_screensaverSettingsIni->value(KEY_CAN_USER_SWITCH, false).toBool();
        m_enableAnimation = m_screensaverSettingsIni->value(KEY_ENABLE_ANIMATION, true).toBool();
        m_lockerPluginPath = m_screensaverSettingsIni->value(KEY_SCREENSAVER_LOCKER, QString(SCREENSAVER_PLUGIN_DIR) + "/libkiran-screensaver-dialog.so").toString();
        m_screensaverTheme = m_screensaverSettingsIni->value(KEY_SCREENSAVER_THEME, "").toString();
    }

    if (!m_powerSettings)
    {
        m_enableDisplayIdleDimmed = m_screensaverSettingsIni->value( KEY_ENABLE_DISPLAY_IDLE_DIMMED, false).toBool();
    }

    if (!m_appearanceSettings)
    {
        m_lockScreenBackground = m_screensaverSettingsIni->value(KEY_LOCK_SCREEN_BACKGROUND, DEFAULT_LOCK_SCREEN_BACKGROUND).toString();
    }
}

void Prefs::setupIniFileWatcher()
{
    if (m_iniFileWatcher)
        return;

    m_iniFileWatcher = new QFileSystemWatcher(this);

    // 文件存在时才添加监控
    if (QFile::exists(SCREENSAVER_CONFIG_FILE))
    {
        m_iniFileWatcher->addPath(SCREENSAVER_CONFIG_FILE);
    }

    connect(m_iniFileWatcher, &QFileSystemWatcher::fileChanged, this, &Prefs::handleIniFileChanged);
}

void Prefs::handleIniFileChanged()
{
    
    // 延迟一下，等文件写完
    QTimer::singleShot(500, this, [this]() {
        // 重新检查文件是否存在
        if (!QFile::exists(SCREENSAVER_CONFIG_FILE))
        {
            KLOG_WARNING() << "ini file not found, skipping reload";
            return;
        }

        // 重新添加监控（文件被删除后重建的情况）
        if (!m_iniFileWatcher->files().contains(SCREENSAVER_CONFIG_FILE))
        {
            m_iniFileWatcher->addPath(SCREENSAVER_CONFIG_FILE);
        }

        m_screensaverSettingsIni->sync();

        // screensaver配置变化检测
        if (!m_screensaverSettings)
        {
            bool oldIdleLock = m_idleActivationLock;
            m_idleActivationLock = m_screensaverSettingsIni->value(KEY_IDLE_ACTIVATION_LOCK, true).toBool();
            if (m_idleActivationLock != oldIdleLock) emit idleActivationLockChanged();

            bool oldIdleSs = m_idleActivationScreensaver;
            m_idleActivationScreensaver = m_screensaverSettingsIni->value(KEY_IDLE_ACTIVATION_SCREENSAVER, true).toBool();
            if (m_idleActivationScreensaver != oldIdleSs) emit idleActivationScrensaverChanged();

            bool oldSplit = m_splitScreensaverAndLock;
            m_splitScreensaverAndLock = m_screensaverSettingsIni->value(KEY_SPLIT_SCREENSAVER_AND_LOCK, false).toBool();
            if (m_splitScreensaverAndLock != oldSplit) emit idleActivationScrensaverChanged();

            m_enableAnimation = m_screensaverSettingsIni->value(KEY_ENABLE_ANIMATION, true).toBool();

            m_screensaverTheme = m_screensaverSettingsIni->value(KEY_SCREENSAVER_THEME, "").toString();
        }

        // power配置
        if (!m_powerSettings)
        {
            bool oldDimmed = m_enableDisplayIdleDimmed;
            m_enableDisplayIdleDimmed = m_screensaverSettingsIni->value(KEY_ENABLE_DISPLAY_IDLE_DIMMED, false).toBool();
            if (m_enableDisplayIdleDimmed != oldDimmed) emit enableIdleDimmedChanged();
        }

        // appearance配置
        if (!m_appearanceSettings)
        {
            QString oldBg = m_lockScreenBackground;
            m_lockScreenBackground = m_screensaverSettingsIni->value(KEY_LOCK_SCREEN_BACKGROUND, DEFAULT_LOCK_SCREEN_BACKGROUND).toString();
            if (m_lockScreenBackground != oldBg) emit lockScreenBackgroundChanged(m_lockScreenBackground);
        }
    });
}

bool Prefs::getIdleActivationLock() const
{
    return m_idleActivationLock;
}

bool Kiran::ScreenSaver::Prefs::getIdleActivationScreensaver() const
{
    if (m_splitScreensaverAndLock)
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
    if (m_screensaverSettings)
    {
        m_screensaverSettings->set(KEY_IDLE_ACTIVATION_LOCK, idleActivationLock);
    }
    else if (m_screensaverSettingsIni)
    {
        m_screensaverSettingsIni->setValue(KEY_IDLE_ACTIVATION_LOCK, idleActivationLock);
    }
}

void Prefs::setCanLogout(bool canLogout)
{
    RETURN_IF_SAME(m_canLogout, canLogout);
    m_canLogout = canLogout;
    if (m_screensaverSettings)
    {
        m_screensaverSettings->set(KEY_CAN_LOGOUT, canLogout);
    }
    else if (m_screensaverSettingsIni)
    {
        m_screensaverSettingsIni->setValue(KEY_CAN_LOGOUT, canLogout);
    }
}

void Prefs::setCanUserSwitch(bool canUserSwitch)
{
    RETURN_IF_SAME(m_canUserSwitch, canUserSwitch);
    m_canUserSwitch = canUserSwitch;
    if (m_screensaverSettings)
    {
        m_screensaverSettings->set(KEY_CAN_USER_SWITCH, canUserSwitch);
    }
    else if (m_screensaverSettingsIni)
    {
        m_screensaverSettingsIni->setValue(KEY_CAN_USER_SWITCH, canUserSwitch);
    }
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
    else if (key == KEY_SPLIT_SCREENSAVER_AND_LOCK)
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
    if (m_screensaverSettings)
    {
        m_screensaverSettings->set(KEY_ENABLE_ANIMATION, enableAnimation);
    }
    else if (m_screensaverSettingsIni)
    {
        m_screensaverSettingsIni->setValue(KEY_ENABLE_ANIMATION, enableAnimation);
    }
}

void Kiran::ScreenSaver::Prefs::handlePowerGSettingsChanged(const QString& key)
{
    if (key == KEY_ENABLE_DISPLAY_IDLE_DIMMED)
    {
        m_enableDisplayIdleDimmed = m_powerSettings->get(key).toBool();
        KLOG_INFO() << "settings changed:" << key << m_enableDisplayIdleDimmed;
        emit enableIdleDimmedChanged();
    }
}

void Prefs::handleAppearanceGSettingsChanged(const QString& key)
{
    if (key == KEY_LOCK_SCREEN_BACKGROUND)
    {
        m_lockScreenBackground = m_appearanceSettings->get(key).toString();
        KLOG_INFO() << "settings changed:" << key << m_lockScreenBackground;
        emit lockScreenBackgroundChanged(m_lockScreenBackground);
    }
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

QString Prefs::getLockScreenBackground() const
{
    return m_lockScreenBackground;
}

bool Kiran::ScreenSaver::Prefs::getEnableDisplayIdleDimmed() const
{
    return m_enableDisplayIdleDimmed;
}
