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

#ifndef KIRAN_SCREENSAVER_SRC_KS_PREFS_H_
#define KIRAN_SCREENSAVER_SRC_KS_PREFS_H_

#include <QtGlobal>
#include <QObject>
#include <QString>
#include <QGSettings>

/**
 * @brief: 提供kiran-screensaver相关配置项的获取与监控
 */
class KSPrefs : public QObject
{
    Q_OBJECT
public:
    KSPrefs(QObject* parent = nullptr);
    ~KSPrefs();

public:
    bool init();

public:
    bool getIdleActivationLock() const;
    bool getCanLogout() const;
    bool getCanUserSwitch() const;
    bool getEnableAnimation() const;

private:
    void setIdleActivationLock(bool idleActivationLock);
    void setCanLogout(bool canLogout);
    void setCanUserSwitch(bool canUserSwitch);
    void setEnableAnimation(bool enableAnimation);

private slots:
    void handleGSettingsChanged(const QString& key);

private:
    QGSettings* m_screensaverSettings = nullptr;
    bool m_idleActivationLock = false;        //空闲时是否锁定
    bool m_canLogout = false;                 //是否允许注销
    bool m_canUserSwitch = false;             //是否允许用户切换
    bool m_enableAnimation = false;           //是否启用动画
};

#endif  //KIRAN_SCREENSAVER_SRC_KS_PREFS_H_
