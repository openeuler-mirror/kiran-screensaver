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

#ifndef KIRAN_SCREENSAVER_SRC_MANAGER_H_
#define KIRAN_SCREENSAVER_SRC_MANAGER_H_

#include <QObject>

class KiranAdaptor;
namespace Kiran
{
namespace ScreenSaver
{
class Prefs;
class Fade;
class Grab;
class Listener;
class ScreenManager;
class IdleWatcher;
class Manager : public QObject
{
    Q_OBJECT
public:
    Manager();
    ~Manager();

public:
    bool init();
    
private:
    bool initDBusListener();
    bool initIdleWatcher();

private slots:
    void onWatcherIdleChanged(bool idle, bool& handled);
    void onWatcherIdleNoticeChanged(bool isEffect, bool& handled);
    void onListenerActiveChanged(bool active, bool& handled);
    void handleListenerLock();
    void processIdleTriggerChanged();
    
private:
    Prefs* m_prefs = nullptr;
    Fade* m_fade = nullptr;
    Grab* m_grab = nullptr;
    IdleWatcher* m_idleWatcher = nullptr;
    Listener* m_listener = nullptr;
    KiranAdaptor* m_kiranAdaptor = nullptr;
    ScreenManager* m_screenManager = nullptr;
};
}  // namespace ScreenSaver
}  // namespace Kiran
#endif  //KIRAN_SCREENSAVER_SRC_MANAGER_H_
