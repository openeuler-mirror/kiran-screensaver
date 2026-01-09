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

#ifndef KIRAN_SCREENSAVER_SRC_GRAB_GRAB_H_
#define KIRAN_SCREENSAVER_SRC_GRAB_GRAB_H_

#include <QWindow>


class QTimer;
namespace Kiran
{
namespace ScreenSaver
{
class InvisibleWindow;
/**
 * @brief 封装的X11输入设备抓取接口
 */
class Grab
{
public:
    static Grab* getInstance();
    ~Grab();

private:
    Grab();
    Grab(Grab& other) = delete;
    Grab& operator=(Grab& other) = delete;

public:
    // 释放抓取
    void releaseGrab();
    // 抓取输入设备至窗口上
    bool grabWindow(WId wid, bool grabPointer = true);
    // 抓取输入设备至root窗口上
    bool grabRoot(bool grabPointer = true);
    // 将输入设备抓取至不显示的窗口上
    bool grabOffscreen(bool grabPointer = true);

private:
    QString getGrabError(int errCode);
    bool doGrab(WId wid,bool grabPointer = true);

private:
    InvisibleWindow* m_invisibleWindow = nullptr;
    WId m_grabWID = 0;
    QTimer* m_recreateInvisibleWindowTimer;
};
}  // namespace ScreenSaver
}  // namespace Kiran

#endif  //KIRAN_SCREENSAVER_SRC_GRAB_GRAB_H_
