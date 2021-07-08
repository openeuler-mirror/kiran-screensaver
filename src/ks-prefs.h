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

enum ScreenSaverMode{
    MODE_BUILD_IN = 0,
    MODE_EMBED,
    MODE_LAST
};

class KSPrefs : public QObject
{
    Q_OBJECT
public:
    KSPrefs(QObject* parent = nullptr);
    ~KSPrefs();

public:
    bool init();

public:
    bool getIdleActivationScreensaver();
    bool getScreensaverActivationLock();
    bool getCanLogout();
    bool getCanLock();
    bool getCanUserSwitch();
    uint getScreenSaverMode();
    QString getScreenSaverTheme();

private:
    void setIdleActivationScreensaver(bool idleActivationScreensaver);
    void setScreensaverActivationLock(bool screensaverActivationLock);
    void setCanLogout(bool canLogout);
    void setCanLock(bool canLock);
    void setCanUserSwitch(bool canUserSwitch);
    void setScreenSaverMode(uint mode);
    void setScreenSaverTheme(QString theme);

private slots:
    void handleGSettingsChanged(const QString& key);

private:
    QGSettings* m_screensaverSettings = nullptr;
    bool m_idleActivationScreensaver = false; //空闲时是否激活屏幕保护程序
    bool m_screensaverActivationLock = false; //屏保激活时锁定屏幕
    bool m_canLogout = false;                 //是否允许注销
    bool m_canLock = false;                   //禁用锁屏
    bool m_canUserSwitch = false;             //是否允许用户切换
    uint m_screensaverMode = 0;               //屏保模式
    QString m_screenSaverTheme;               //屏保显示主题,当使用内置屏保时,该值被忽略
};

#endif  //KIRAN_SCREENSAVER_SRC_KS_PREFS_H_
