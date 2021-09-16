/**
 * @file   ks-listener.h
 * @biref  dbus服务具体实现
 * @author liuxinhao <liuxinhao@kylinos.com.cn>
 * @copyright (c) 2021 KylinSec. All rights reserved. 
 */
#ifndef KIRAN_SCREENSAVER_SRC_KS_LISTENER_H_
#define KIRAN_SCREENSAVER_SRC_KS_LISTENER_H_

#include <QObject>
#include <QDBusContext>
#include <QDebug>

class LogindSessionMonitor;
/**
 * @brief 对外提供DBus接口
 * 对内提供接口整合设置屏保状态和空闲状态
 */
class KSListener: public QObject, protected QDBusContext
{
    Q_OBJECT
public:
    struct InhibitedEntry{
        QString application;
        QString reason;
        QString connection;
        quint32 cookie;
        quint32 foreign_cookie;
        quint64 since; // 时间戳
        friend QDebug operator<<(QDebug debug, const InhibitedEntry &entry);
    };
    explicit KSListener(QObject* parent = nullptr);
    ~KSListener() override;

    /// 初始化Systemd Login1 DBus服务监听对象
    /// 连接到Login1获取Lock,Unlock信号，同步调用ScreenSaver接口
    /// \return
    bool init();

    /// 获取当前状态是否是被抑制的状态
    /// \return 是否被抑制
    bool isInhibited();

    /// 设置会话空闲状态
    /// 会判断抑制器是否被阻止，会同步设置屏保激活状态，若激活状态设置失败，则返回值失败
    /// \param idle 会话是否空闲
    /// \return 是否成功(true表示设置会话空闲成功,false设置失败或已是该状态)
    bool setSessionIdle(bool idle);

    /// 设置屏保状态
    /// \param active 屏保激活状态
    /// \return 是否成功
    bool setActiveStatus(bool active);

public slots:
    // 以下方法提供方法只为了兼容mate桌面，并不作任何实际有效的操作
    void Cycle();
    uint Throttle(const QString &application_name, const QString &reason);
    void UnThrottle(uint cookie);
    void ShowMessage(const QString &summary, const QString &body, const QString &icon);

    // 设置激活状态
    void SetActive(bool value);
    // 获取激活状态
    bool GetActive();
    // 获取激活时间
    uint GetActiveTime();

    // 获取抑制器cookie列表
    QStringList GetInhibitors();
    // 添加一个抑制器
    uint Inhibit(const QString &application_name, const QString &reason);
    // 删除一个抑制器
    void UnInhibit(uint cookie);

    // 请求锁定屏幕
    void Lock();
    // 请求解锁屏幕
    void Unlock();

    // 模拟用户活动
    void SimulateUserActivity();

signals:
    // DBus信号 - 屏幕保护程序激活状态
    void ActiveChanged(bool value);

    // 内部处理信号 - 处理激活状态变更信号
    void sigActiveChanged(bool active,bool &handled);

    // 内部处理信号 - 激活锁屏
    void sigLock();

    // 内部处理信号 - 模拟用户活动
    void sigSimulateUserActivity();

private:
    // 添加抑制器，改动同步至会话管理中
    void addInhibitEntry(InhibitedEntry &entry);
    // 删除抑制器，改动同步至会话管理中
    void removeInhibitEntry(quint64 cookie);

    // 添加空闲抑制器到会话管理中
    void addSessionInhibit(InhibitedEntry& entry);
    // 删除会话管理中的空闲抑制器
    void removeSessionInhibit(const InhibitedEntry& entry);

    quint64 generateCookie();


private slots:
    // dbus服务logind发出Lock信号的槽函数
    void handleLogindSessionLock();
    // dbus服务logind发出UnLock信号的槽函数
    void handleLogindSessionUnlock();

    void handleDBusNameOwnerChanged(const QString& name,const QString& oldOwner,const QString& newOwner);

private:
    bool m_isActive = false;
    bool m_sessionIdle = false;
    uint m_activeStart = 0;
    uint m_sessionIdleStart = 0;
    LogindSessionMonitor* m_sessionMonitor = nullptr;
    QMap<quint64,InhibitedEntry> m_inhibitedEntries;
};

#endif  //KIRAN_SCREENSAVER_SRC_KS_LISTENER_H_
