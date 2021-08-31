/**
 * @file   ks-prefs.h
 * @biref  KiranScreenSaver配置项
 * @author liuxinhao <liuxinhao@kylinos.com.cn>
 * @copyright (c) 2021 KylinSec. All rights reserved. 
 */
#ifndef KIRAN_SCREENSAVER_SRC_KS_PREFS_H_
#define KIRAN_SCREENSAVER_SRC_KS_PREFS_H_

#include <QtGlobal>
#include <QObject>
#include <QString>
#include <QGSettings>

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
