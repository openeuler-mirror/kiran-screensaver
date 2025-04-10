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

#include "grab.h"
#include "invisible-window.h"
#include "xcb-utils.h"
#include <qt5-log-i.h>
#include <xcb/xproto.h>
#include <QMap>
#include <QMutex>
#include <QScopedPointer>
#include <QX11Info>
#include <memory>
#include <unistd.h>
#include <QTimer>

using namespace Kiran::ScreenSaver;

Grab* Grab::getInstance()
{
    static QMutex mutex;
    static QScopedPointer<Grab> pInst;

    if (Q_UNLIKELY(!pInst))
    {
        QMutexLocker locker(&mutex);
        if (pInst.isNull())
        {
            pInst.reset(new Grab);
        }
    }

    return pInst.data();
}

Grab::~Grab()
{
    delete m_recreateInvisibleWindowTimer;
}

Grab::Grab()
{
    m_invisibleWindow = new InvisibleWindow;
    m_invisibleWindow->show();

    //NOTE:
    // #15523
    // xsmp  SmSaveGlobal 询问退出阶段, QGuiApplicationPrivate::commitData会尝试关闭所有窗口
    // 该点会导致invisible window close
    // 通过感知close事件后，重新拉取invisible window避免注销取消后，无法抓取输入到离屏窗口之上的问题
    m_recreateInvisibleWindowTimer = new QTimer;
    m_recreateInvisibleWindowTimer->setSingleShot(true);
    m_recreateInvisibleWindowTimer->setInterval(0);

    QObject::connect(m_recreateInvisibleWindowTimer,&QTimer::timeout,[this](){
        this->m_invisibleWindow->show();
    });

    QObject::connect(m_invisibleWindow,&InvisibleWindow::windowClosed,[this](){
        this->m_recreateInvisibleWindowTimer->start();
    });
}

void Grab::releaseGrab()
{
    KLOG_DEBUG() << "release grab";
    xcb_ungrab_keyboard(QX11Info::connection(), XCB_TIME_CURRENT_TIME);
    xcb_ungrab_pointer(QX11Info::connection(), XCB_TIME_CURRENT_TIME);
}

bool Grab::grabWindow(WId wid, bool grabPointer)
{
    int retries = 12;

    releaseGrab();

    bool grabbed = false;
    for (int i = 0; i < retries; i++)
    {
        grabbed = doGrab(wid,grabPointer);
        if( grabbed )
        {
            break;
        }
        sleep(1);
    }

    return grabbed;
}

bool Grab::grabRoot(bool grabPointer)
{
    KLOG_DEBUG() << "grab to root window";
    return grabWindow(QX11Info::appRootWindow(), grabPointer);
}

// NOTE:若抓取失败，可以考虑模拟出Esc按键，取消掉开始菜单的抓取
bool Grab::grabOffscreen(bool grabPointer)
{
    KLOG_DEBUG() << "grab to offscreen window";
    return grabWindow(m_invisibleWindow->winId(), grabPointer);
}

QString Grab::getGrabError(int errCode)
{
    static const QMap<int, QString> GrabStatusDescMap = {
        {XCB_GRAB_STATUS_SUCCESS, "XCB_GRAB_STATUS_SUCCESS"},
        {XCB_GRAB_STATUS_ALREADY_GRABBED, "XCB_GRAB_STATUS_ALREADY_GRABBED"},
        {XCB_GRAB_STATUS_INVALID_TIME, "XCB_GRAB_STATUS_INVALID_TIME"},
        {XCB_GRAB_STATUS_NOT_VIEWABLE, "XCB_GRAB_STATUS_NOT_VIEWABLE"},
        {XCB_GRAB_STATUS_FROZEN, "XCB_GRAB_STATUS_FROZEN"}};

    auto iter = GrabStatusDescMap.find(errCode);
    return iter == GrabStatusDescMap.end() ? "XCB_GRAB_STATUS_OTHER" : iter.value();
}

bool Grab::doGrab(WId wid, bool grabPointer)
{
    XServerGrabber serverGrabber;
    bool bRes = false;

    // grab keyboard
    auto reply = KS_XCB_REPLY(xcb_grab_keyboard,
                              QX11Info::connection(),
                              true,
                              wid,
                              XCB_TIME_CURRENT_TIME,
                              XCB_GRAB_MODE_ASYNC,
                              XCB_GRAB_MODE_ASYNC);

    bRes = reply && reply->status == XCB_GRAB_STATUS_SUCCESS;
    if (!bRes)
    {
        int errCode = reply ? reply->status:-1;
        KLOG_WARNING() << "grab keyboard to" << wid << "failed!" << getGrabError(errCode);
        return bRes;
    }

    // grab pointer
    if (grabPointer)
    {
        auto reply = KS_XCB_REPLY(xcb_grab_pointer,
                                  QX11Info::connection(),
                                  false,
                                  wid,
                                  (XCB_EVENT_MASK_BUTTON_PRESS | XCB_EVENT_MASK_BUTTON_RELEASE |
                                   XCB_EVENT_MASK_BUTTON_MOTION | XCB_EVENT_MASK_POINTER_MOTION |
                                   XCB_EVENT_MASK_ENTER_WINDOW | XCB_EVENT_MASK_LEAVE_WINDOW),
                                  XCB_GRAB_MODE_ASYNC,
                                  XCB_GRAB_MODE_ASYNC,
                                  XCB_WINDOW_NONE,
                                  XCB_CURSOR_NONE,
                                  XCB_TIME_CURRENT_TIME);
        bRes = reply && reply->status == XCB_GRAB_STATUS_SUCCESS;
        if (!bRes)
        {
            int errCode = reply ? reply->status:1;
            KLOG_WARNING() << "grab pointer to" << wid << "failed!" << getGrabError(errCode);
            xcb_ungrab_keyboard(QX11Info::connection(), XCB_TIME_CURRENT_TIME);
        }
    }

    return bRes;
}