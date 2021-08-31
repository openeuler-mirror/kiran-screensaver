 /**
  * @Copyright (C) 2020 ~ 2021 KylinSec Co., Ltd.
  *
  * Author:     liuxinhao <liuxinhao@kylinos.com.cn>
  *
  * This program is free software; you can redistribute it and/or modify
  * it under the terms of the GNU General Public License as published by
  * the Free Software Foundation; either version 3 of the License, or
  * (at your option) any later version.
  *
  * This program is distributed in the hope that it will be useful,
  * but WITHOUT ANY WARRANTY; without even the implied warranty of
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  * GNU General Public License for more details.
  *
  * You should have received a copy of the GNU General Public License
  * along with this program; If not, see <http: //www.gnu.org/licenses/>. 
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
    void handleIdleChanged(bool idle,bool &handled);
    void handleIdleNoticeChanged(bool isEffect,bool &handled);
    void handleListenerActiveChanged(bool active,bool &handled);
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
