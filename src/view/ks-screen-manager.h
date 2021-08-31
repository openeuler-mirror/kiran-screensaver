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
 
#ifndef KIRAN_SCREENSAVER_SRC_SCREEN_MANAGER_KS_SCREEN_MANAGER_H_
#define KIRAN_SCREENSAVER_SRC_SCREEN_MANAGER_KS_SCREEN_MANAGER_H_

#include <QMap>
#include <QObject>
#include <QPixmap>

class KSWindow;
class QScreen;
class QGSettings;
class KSFade;
class KSScreensaver;
class KSLockerDemo;
class KSScreenManager : public QObject
{
    Q_OBJECT
public:
    KSScreenManager(KSFade* fade,QObject* parent = nullptr);
    ~KSScreenManager();

    bool eventFilter(QObject* watched, QEvent* event) override;

public:
    /// 设置激活状态
    bool setActive(bool visible);
    /// 获取激活状态
    bool getActive();

    void setLockActive(bool lockActive);
    bool getLockActive();
    bool requestUnlock();

private:
    bool activate();
    void createWindows();
    void createWindowForScreen(QScreen* screen);

    bool deactivate();
    void destroyWindows();

private slots:
    void handleScreenAdded(QScreen* screen);
    void handleScreenRemoved(QScreen* screen);

private:
    bool m_active = false;
    bool m_lockActive = false;
    bool m_dialogUp = false;
    KSFade * m_fade = nullptr;

    QPixmap m_background;

    QMap<QScreen* ,KSWindow*> m_windowMap;
    KSScreensaver* m_screensaver;
    KSLockerDemo* m_lockerDemo;
};

#endif  //KIRAN_SCREENSAVER_SRC_SCREEN_MANAGER_KS_SCREEN_MANAGER_H_
