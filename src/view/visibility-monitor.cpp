/**
 * Copyright (c) 2020 ~ 2024 KylinSec Co., Ltd.
 * kiran-screensaver is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 *
 * Author:     liuxinhao <liuxinhao@kylinsec.com.cn>
 */
#include "visibility-monitor.h"
#include <qt5-log-i.h>
#include <xcb/xcb.h>
#include <QMutex>
#include <QScopedPointer>
#include <QSocketNotifier>
#include "xcb-utils.h"

namespace Kiran
{
namespace ScreenSaver
{
VisibilityMonitor* VisibilityMonitor::instance()
{
    static QMutex mutex;
    static QScopedPointer<VisibilityMonitor> _instance;

    if (Q_UNLIKELY(!_instance))
    {
        QMutexLocker locker(&mutex);
        if (_instance.isNull())
        {
            _instance.reset(new VisibilityMonitor());
        }
    }

    return _instance.data();
}

VisibilityMonitor::~VisibilityMonitor()
{
    if (m_xcbConnection != nullptr)
    {
        xcb_disconnect(m_xcbConnection);
    }
}

void VisibilityMonitor::monitor(WId wid)
{
    if (m_windows.contains(wid))
    {
        return;
    }

    KLOG_INFO() << "visibility monitor:" << wid;

    // 关闭混成时，XServer发出VisibilityNotify事件
    // 直接订阅该事件用来监控窗口可见状态更高效
    uint32_t valueList[] = {XCB_EVENT_MASK_VISIBILITY_CHANGE};
    xcb_change_window_attributes(m_xcbConnection, wid, XCB_CW_EVENT_MASK, valueList);

    // 订阅用来处理混成顺序的事件
    // ConfigureNotify 以及 MapNotify
    if (m_windows.isEmpty())
    {
        selectSubstructureNotify();
    }
    xcb_flush(m_xcbConnection);
    m_windows << wid;
}

void VisibilityMonitor::unmonitor(WId wid)
{
    if (!m_windows.contains(wid))
    {
        return;
    }

    KLOG_INFO() << "visibility unmonitor:" << wid;

    uint32_t valueList[] = {XCB_EVENT_MASK_NO_EVENT};
    xcb_change_window_attributes(m_xcbConnection, wid, XCB_CW_EVENT_MASK, valueList);

    m_windows.remove(wid);
    if (m_windows.isEmpty())
    {
        unselectSubstructureNotify();
    }
    xcb_flush(m_xcbConnection);
}

VisibilityMonitor::VisibilityMonitor(QObject* parent)
    : QObject(parent)
{
    init();
    xcb_connect(nullptr, nullptr);
}

void VisibilityMonitor::init()
{
    if (m_inited)
        return;

    m_xcbConnection = xcb_connect(nullptr, nullptr);
    if (xcb_connection_has_error(m_xcbConnection))
    {
        KLOG_WARNING() << "visibility monitor xcb_connect failed";
        xcb_disconnect(m_xcbConnection);
        m_xcbConnection = nullptr;
        return;
    }

    int fd = xcb_get_file_descriptor(m_xcbConnection);
    m_xcbSocketNotifier = new QSocketNotifier(fd, QSocketNotifier::Read, this);
    connect(m_xcbSocketNotifier, &QSocketNotifier::activated, this,
            &VisibilityMonitor::onXcbSocketNotifierActivated);

    KLOG_INFO() << "visibility monitor inited";
    m_inited = true;
}

bool VisibilityMonitor::isInternal(WId window)
{
    bool bRes = false;
    if (m_windows.contains(window) && window != QX11Info::appRootWindow())
    {
        bRes = true;
    }
    return bRes;
}

void VisibilityMonitor::selectSubstructureNotify()
{
    WId root = QX11Info::appRootWindow();

    auto reply = KS_XCB_REPLY(xcb_get_window_attributes, m_xcbConnection, root);
    if (reply == nullptr)
    {
        KLOG_WARNING() << "select SubstructureNotify failed,can't get root window attributes";
        return;
    }
    if (reply->your_event_mask & XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY)
    {
        KLOG_DEBUG() << "SubstructureNotify already selected";
        return;
    }

    uint32_t values[] = {XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY};
    // uint32_t values[] = {XCB_EVENT_MASK_VISIBILITY_CHANGE};
    xcb_change_window_attributes(m_xcbConnection, root,
                                 XCB_CW_EVENT_MASK, values);

    KLOG_INFO() << "select root window substructure notify event.";
}

void VisibilityMonitor::unselectSubstructureNotify()
{
    WId root = QX11Info::appRootWindow();

    auto reply = KS_XCB_REPLY(xcb_get_window_attributes, m_xcbConnection, root);
    if (reply == nullptr)
    {
        KLOG_WARNING() << "unselect SubstructureNotify failed,can't get root window attributes";
        return;
    }

    if (!(reply->your_event_mask & XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY))
    {
        KLOG_DEBUG() << "SubstructureNotify already unselected";
        return;
    }

    uint32_t values[] = {XCB_EVENT_MASK_NO_EVENT};
    xcb_change_window_attributes(m_xcbConnection, root,
                                 XCB_CW_EVENT_MASK, values);

    KLOG_INFO() << "unselect root window substructure notify event.";
}

void VisibilityMonitor::handleXcbEvent()
{
    int count = 0;
    while (xcb_generic_event_t* event = xcb_poll_for_event(m_xcbConnection))
    {
        switch (event->response_type & ~0x80)
        {
        case XCB_VISIBILITY_NOTIFY:
            onVisibilityNotify(event);
            break;
        case XCB_MAP_NOTIFY:
            onMapNotify(event);
            break;
        case XCB_CONFIGURE_NOTIFY:
            onConfigureNotify(event);
            break;
        default:
            break;
        }
        ::free(event);
        count++;
    }

    if (count)
        xcb_flush(m_xcbConnection);
}

void VisibilityMonitor::onVisibilityNotify(xcb_generic_event_t* event)
{
    xcb_visibility_notify_event_t* visibilityEvent = reinterpret_cast<xcb_visibility_notify_event_t*>(event);
    uint8_t visibilityState = visibilityEvent->state;

    if ((visibilityState >= VISIBILITY_UNOBSCURED) && (visibilityState < VISIBILITY_LAST))
    {
        VisibilityState eState = (VisibilityState)visibilityState;
        KLOG_INFO() << "visibility state changed:"
                    << visibilityEvent->window
                    << "->" << eState;
        emit visibilityStateChanged(visibilityEvent->window, eState);
    }
    else
    {
        KLOG_WARNING() << "unknow visibility state:" << visibilityEvent->window << visibilityState;
    }
}

void VisibilityMonitor::onMapNotify(xcb_generic_event_t* event)
{
    xcb_map_notify_event_t* mapEvent = reinterpret_cast<xcb_map_notify_event_t*>(event);

    if (isInternal(mapEvent->window))
    {
        return;
    }

    KLOG_DEBUG() << "VisibilityMonitor: MapNotify" << mapEvent->window;
    emit restackedNeedRaise();
}

void VisibilityMonitor::onConfigureNotify(xcb_generic_event_t* event)
{
    xcb_configure_notify_event_t* configureEvent = reinterpret_cast<xcb_configure_notify_event_t*>(event);

    if (isInternal(configureEvent->window))
    {
        return;
    }

    KLOG_DEBUG() << "VisibilityMonitor: ConfigureNotify" << configureEvent->window;
    emit restackedNeedRaise();
}

#if (QT_VERSION >= QT_VERSION_CHECK(5, 15, 0))
void VisibilityMonitor::onXcbSocketNotifierActivated(QSocketDescriptor socket,
                                                     QSocketNotifier::Type activationEvent)
{
    if (activationEvent != QSocketNotifier::Read)
    {
        return;
    }

    handleXcbEvent();
}
#else
void VisibilityMonitor::onXcbSocketNotifierActivated(int socket)
{
    handleXcbEvent();
}
#endif
}  // namespace ScreenSaver
}  // namespace Kiran