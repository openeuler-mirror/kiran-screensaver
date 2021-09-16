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
#include "gsm_presence_proxy.h"

/**
 * @brief 空闲监控通过mate-session
 */
class KSIdleWatcherMate : public KSIdleWatcher
{
    Q_OBJECT
public:
    explicit KSIdleWatcherMate(QObject* parent = nullptr);
    ~KSIdleWatcherMate();

public:
    bool init() override;

    bool getIdleDetectionActive() override;
    bool setIdleDetectionActive(bool idleDetectionActive) override;

    bool getEnabled() override;
    bool setEnabled(bool enabled) override;

private slots:
    // 连接到gnome session manager Presence监听整个桌面的空闲状态
    void slotPresenceStatusChanged(uint status);
    // 连接到gnome session manager Presence获取状态文本的改变
    void slotPresenceStatusTextChanged(const QString& statusText);
    // 处理空闲状态的定时器超时,由空闲预告进入空闲的阶段
    void slotHandleIdleDelayTimeout();

private:
    void setStatus(uint status);
    bool setIdle(bool idle);
    bool setIdleNotice(bool notice);

protected:
    void timerEvent(QTimerEvent* event) override;

private:
    GSMPresenceProxy* m_presenceInterface = nullptr;

    ///---设置---
    //是否启用
    bool m_enabled = false;
    //延迟通知空闲超时时间
    uint64_t m_delayIdleTimeout = 2000;

    //空闲检测是否开启
    bool m_idleDetectionActive = false;
    //空闲状态是否开启
    bool m_idle = false;
    //空闲预告标志
    bool m_idleNotice = false;

    // 延迟通知空闲定时器
    int m_idleTimerID = 0;
    // 定时禁用xscreensaver定时器
    int m_disableXScreenSaverID = 0;

    // 来自于mate-session presence status-message
    QString m_statusMsg;
};

#endif  //KIRAN_SCREENSAVER_SRC_KS_IDLE_WATCHER_MATE_H_
