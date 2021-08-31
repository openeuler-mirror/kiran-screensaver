//
// Created by lxh on 2021/7/6.
//

#ifndef KIRAN_SCREENSAVER_SRC_LISTENER_LOGIND_SESSION_MONITOR_H_
#define KIRAN_SCREENSAVER_SRC_LISTENER_LOGIND_SESSION_MONITOR_H_

#include <QObject>

// 监听logind发出的Lock、UnLock信号的封装
class LogindSessionMonitor : public QObject
{
    Q_OBJECT
public:
    explicit LogindSessionMonitor(QObject* parent = nullptr);
    ~LogindSessionMonitor() override;

public:
    bool init();

signals:
    void Lock();
    void Unlock();
};

#endif  //KIRAN_SCREENSAVER_SRC_LISTENER_LOGIND_SESSION_MONITOR_H_
