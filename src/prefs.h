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
#ifndef KIRAN_SCREENSAVER_SRC_PREFS_H_
#define KIRAN_SCREENSAVER_SRC_PREFS_H_

#include <QGSettings>
#include <QObject>
#include <QString>
#include <QtGlobal>
#include "config.h"

/**
 * @brief: 提供kiran-screensaver相关配置项的获取与监控
 */
namespace Kiran
{
namespace ScreenSaver
{
class Prefs : public QObject
{
    Q_OBJECT
public:
    static Prefs* getInstance();
    ~Prefs() override;

private:
    Prefs(QObject* parent = nullptr);

public:
    bool init();

public:
    bool getIdleActivationLock() const;
    bool getIdleActivationScreensaver() const;

    bool getCanLogout() const;
    bool getCanUserSwitch() const;
    bool getEnableAnimation() const;
    QString getScreensaverTheme() const;
    QString getLockerPluginPath() const;

    bool getEnableDisplayIdleDimmed() const;
signals:
    void idleActivationLockChanged();
    void idleActivationScrensaverChanged();
    void enableIdleDimmedChanged();
    
private:
    void setIdleActivationLock(bool idleActivationLock);
    void setCanLogout(bool canLogout);
    void setCanUserSwitch(bool canUserSwitch);
    void setEnableAnimation(bool enableAnimation);

private slots:
    void handleGSettingsChanged(const QString& key);
    void handlePowerGSettingsChanged(const QString& key);

private:
    bool isInited = false;
    QGSettings* m_screensaverSettings = nullptr;

    bool m_splitScreensaverAndLock = false;  // 是否拆分空闲锁定以及屏保配置
    bool m_idleActivationLock = true;        // 空闲时是否锁定/屏保
    bool m_idleActivationScreensaver = true; // 空闲时是否锁定

    bool m_canLogout = false;        // 是否允许注销
    bool m_canUserSwitch = false;    // 是否允许用户.切换
    bool m_enableAnimation = false;  // 是否启用动画
    QString m_lockerPluginPath;      // 解锁框插件位置
    QString m_screensaverTheme;      // 屏保主题

    QGSettings* m_powerSettings = nullptr;
    bool m_enableDisplayIdleDimmed = false;
};
}  // namespace ScreenSaver
}  // namespace Kiran
#endif  // KIRAN_SCREENSAVER_SRC_PREFS_H_
