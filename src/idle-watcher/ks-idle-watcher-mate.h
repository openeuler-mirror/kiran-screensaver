/**
 * @file   ks-idle-watcher-mate.h
 * @biref  适用于mate桌面的空闲监控对象，负责监控桌面空闲状态相关信息
 * @author liuxinhao <liuxinhao@kylinos.com.cn>
 * @copyright (c) 2021 KylinSec. All rights reserved. 
 */
#ifndef KIRAN_SCREENSAVER_SRC_KS_IDLE_WATCHER_MATE_H_
#define KIRAN_SCREENSAVER_SRC_KS_IDLE_WATCHER_MATE_H_

#include "ks-idle-watcher.h"
///由org.gnome.SessionManager.Presence.xml生成而来
#include "gnome_session_presence.h"

class KSIdleWatcherMate : public KSIdleWatcher
{
    Q_OBJECT
public:
    explicit KSIdleWatcherMate(QObject* parent = nullptr);
    ~KSIdleWatcherMate();

public:
    bool init() override;

    bool getActive() override;
    bool setActive(bool active) override;

    bool getEnabled() override;
    bool setEnabled(bool enabled) override;

private slots:
    void slotPresenceStatusChanged(uint status);
    void slotPresenceStatusTextChanged(const QString& statusText);
    void slotHandleIdleDelayTimeout();

private:
    void setStatus(uint status);
    bool setIdle(bool idle);
    bool setIdleNotice(bool notice);

protected:
    void timerEvent(QTimerEvent* event) override;

private:
    org::gnome::SessionManager::Presence* m_presenceInterface = nullptr;

    ///---设置---
    //是否启用
    bool m_enabled = false;
    //延迟通知空闲超时时间
    uint64_t m_delayIdleTimeout = 1000;

    ///---状态---
    //激活状态
    bool m_active = false;
    //空闲状态
    bool m_idle = false;
    //空闲预告标志
    bool m_idleNotice = false;

    //延迟通知空闲定时器
    int m_idleTimerID = 0;
    //定时禁用xscreensaver定时器
    int m_disableXScreenSaverID = 0;

    //来自于mate-session presence status-message
    QString m_statusMsg;
};

#endif  //KIRAN_SCREENSAVER_SRC_KS_IDLE_WATCHER_MATE_H_
