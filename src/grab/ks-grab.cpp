//
// Created by lxh on 2021/7/8.
//

#include "ks-grab.h"
#include "ks-invisible-window.h"

#include <qt5-log-i.h>
#include <xcb/xproto.h>
#include <QMutex>
#include <QScopedPointer>
#include <QX11Info>
#include <memory>

struct FreeDeleter
{
    void operator()(void* pointer) const Q_DECL_NOTHROW
    {
        return std::free(pointer);
    }
};

#define XCB_REPLY_CONNECTION_ARG(connection, ...) connection

#define KS_XCB_REPLY(call, ...) \
    std::unique_ptr<call##_reply_t, FreeDeleter>(call##_reply(XCB_REPLY_CONNECTION_ARG(__VA_ARGS__), call(__VA_ARGS__), nullptr))

KSGrab* KSGrab::getInstance()
{
    static QMutex mutex;
    static QScopedPointer<KSGrab> pInst;

    if (Q_UNLIKELY(!pInst))
    {
        QMutexLocker locker(&mutex);
        if (pInst.isNull())
        {
            pInst.reset(new KSGrab);
        }
    }

    return pInst.data();
}

KSGrab::~KSGrab()
{
    delete m_invisibleWindow;
}

KSGrab::KSGrab()
{
    m_invisibleWindow = new KSInvisibleWindow;
    m_invisibleWindow->show();
}

void KSGrab::releaseGrab()
{
    xcb_ungrab_keyboard(QX11Info::connection(),XCB_TIME_CURRENT_TIME);
    xcb_ungrab_pointer(QX11Info::connection(),XCB_TIME_CURRENT_TIME);
}

bool KSGrab::grabWindow(WId wid, bool grabPointer)
{
    KLOG_DEBUG() << "grab to window" << wid;
    bool bRes = false;

    if (grabPointer)
    {
        auto reply = KS_XCB_REPLY(xcb_grab_pointer,
                                  QX11Info::connection(),
                                  false,
                                  wid,
                                  (XCB_EVENT_MASK_BUTTON_PRESS | XCB_EVENT_MASK_BUTTON_RELEASE | XCB_EVENT_MASK_BUTTON_MOTION | XCB_EVENT_MASK_POINTER_MOTION | XCB_EVENT_MASK_ENTER_WINDOW | XCB_EVENT_MASK_LEAVE_WINDOW),
                                  XCB_GRAB_MODE_ASYNC,
                                  XCB_GRAB_MODE_ASYNC,
                                  XCB_WINDOW_NONE,
                                  XCB_CURSOR_NONE,
                                  XCB_TIME_CURRENT_TIME);
        bRes = reply && reply->status==XCB_GRAB_STATUS_SUCCESS;
        if(!bRes)
        {
            KLOG_DEBUG() << "can't grab,grab status:" << reply->status;
            return false;
        }
    }

    auto reply = KS_XCB_REPLY(xcb_grab_keyboard,
                              QX11Info::connection(),
                              false,
                              wid,
                              XCB_TIME_CURRENT_TIME,
                              XCB_GRAB_MODE_ASYNC,
                              XCB_GRAB_MODE_ASYNC);
    bRes = reply && reply->status==XCB_GRAB_STATUS_SUCCESS;
    if(!bRes && grabPointer)
    {
        KLOG_DEBUG() << "can't grab,grab status:" << reply->status;
        xcb_ungrab_pointer(QX11Info::connection(),XCB_TIME_CURRENT_TIME);
        return false;
    }

    return true;
}

bool KSGrab::grabRoot(bool grabPointer)
{
    return grabWindow(QX11Info::appRootWindow(),grabPointer);
}

bool KSGrab::grabOffscreen(bool grabPointer)
{
    return grabWindow(m_invisibleWindow->winId(),grabPointer);
}