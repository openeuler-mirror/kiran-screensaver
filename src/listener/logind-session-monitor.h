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
#ifndef KIRAN_SCREENSAVER_SRC_LISTENER_LOGIND_SESSION_MONITOR_H_
#define KIRAN_SCREENSAVER_SRC_LISTENER_LOGIND_SESSION_MONITOR_H_

#include <QObject>

// 监听logind发出的Lock、UnLock信号的封装
namespace Kiran
{
namespace ScreenSaver
{
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
}  // namespace ScreenSaver
}  // namespace Kiran
#endif  //KIRAN_SCREENSAVER_SRC_LISTENER_LOGIND_SESSION_MONITOR_H_
