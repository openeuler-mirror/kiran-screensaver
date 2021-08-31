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
#ifndef KIRAN_SCREENSAVER_SRC_GRAB_KS_GRAB_H_
#define KIRAN_SCREENSAVER_SRC_GRAB_KS_GRAB_H_

#include <QWindow>

class KSInvisibleWindow;
class KSGrab
{
public:
    static KSGrab* getInstance();
    ~KSGrab();

private:
    KSGrab();

public:
    // 释放抓取
    void releaseGrab();
    // 抓取输入设备至窗口上
    bool grabWindow(WId wid,bool grabPointer=true);
    // 抓取输入设备至root窗口上
    bool grabRoot(bool grabPointer=true);
    // 将输入设备抓取至不显示的窗口上
    bool grabOffscreen(bool grabPointer=true);

private:
    KSInvisibleWindow* m_invisibleWindow = nullptr;
    WId m_grabWID = 0;
};

#endif  //KIRAN_SCREENSAVER_SRC_GRAB_KS_GRAB_H_
