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


#ifndef KIRAN_SCREENSAVER_SRC_KS_MANAGER_H_
#define KIRAN_SCREENSAVER_SRC_KS_MANAGER_H_

#include <QObject>

class KSPrefs;
class KSFade;
class KSGrab;
class KSListener;
class KSMateAdaptor;
class KSKiranAdaptor;
class KSScreenManager;
class KSIdleWatcher;
class KSManager: public QObject
{
    Q_OBJECT
public:
    KSManager();
    ~KSManager();

public:
    bool init();
    bool initDBusListener();
    bool initIdleWatcher();

private slots:
    void onWatcherIdleChanged(bool idle,bool &handled);
    void onWatcherIdleNoticeChanged(bool isEffect,bool &handled);
    void onListenerActiveChanged(bool active,bool &handled);
    void handleListenerLock();

private:
    KSPrefs* m_prefs = nullptr;
    KSFade* m_fade = nullptr;
    KSGrab* m_grab = nullptr;
    KSIdleWatcher* m_idleWatcher = nullptr;
    KSListener* m_listener = nullptr;
    KSMateAdaptor* m_mateAdaptor = nullptr;
    KSKiranAdaptor* m_kiranAdaptor = nullptr;
    KSScreenManager* m_screenManager = nullptr;
};

#endif  //KIRAN_SCREENSAVER_SRC_KS_MANAGER_H_
