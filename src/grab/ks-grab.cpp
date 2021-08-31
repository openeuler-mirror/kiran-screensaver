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
#include <QMap>

static const QMap<int,QString> GrabStatusDescMap = {
    {XCB_GRAB_STATUS_SUCCESS,"XCB_GRAB_STATUS_SUCCESS"},
    {XCB_GRAB_STATUS_ALREADY_GRABBED,"XCB_GRAB_STATUS_ALREADY_GRABBED"},
    {XCB_GRAB_STATUS_INVALID_TIME,"XCB_GRAB_STATUS_INVALID_TIME"},
    {XCB_GRAB_STATUS_NOT_VIEWABLE,"XCB_GRAB_STATUS_NOT_VIEWABLE"},
    {XCB_GRAB_STATUS_FROZEN,"XCB_GRAB_STATUS_FROZEN"}
};

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

class XServerGrabber
{
public:
    XServerGrabber()
    {
        xcb_grab_server(QX11Info::connection());
    }
    ~XServerGrabber()
    {
        xcb_ungrab_server(QX11Info::connection());
        xcb_flush(QX11Info::connection());
    }
};

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
//    delete m_invisibleWindow;
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

    XServerGrabber serverGrabber;

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
            auto iter = GrabStatusDescMap.find(reply->status);
            KLOG_DEBUG() << "can't grab,grab status:" << reply->status << (iter->isEmpty()?"":iter.value());
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
        auto iter = GrabStatusDescMap.find(reply->status);
        KLOG_DEBUG() << "can't grab,grab status:" << reply->status << (iter->isEmpty()?"":iter.value());
        xcb_ungrab_pointer(QX11Info::connection(),XCB_TIME_CURRENT_TIME);
        return false;
    }

    return true;
}

bool KSGrab::grabRoot(bool grabPointer)
{
    return grabWindow(QX11Info::appRootWindow(),grabPointer);
}

//NOTE:若抓取失败，可以考虑模拟出Esc按键，取消掉开始菜单的抓取
bool KSGrab::grabOffscreen(bool grabPointer)
{
    return grabWindow(m_invisibleWindow->winId(),grabPointer);
}