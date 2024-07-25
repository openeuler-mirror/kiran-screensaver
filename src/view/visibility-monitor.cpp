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

void VisibilityMonitor::monitorWindow(WId wid)
{
    KLOG_INFO() << "visibility monitor:" << wid;
    uint32_t valueList[] = {XCB_EVENT_MASK_VISIBILITY_CHANGE};
    xcb_change_window_attributes(m_xcbConnection, wid, XCB_CW_EVENT_MASK, valueList);
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

void VisibilityMonitor::handleXcbEvent()
{
    int count = 0;
    while (xcb_generic_event_t* event = xcb_poll_for_event(m_xcbConnection))
    {
        switch (event->response_type & ~0x80)
        {
        case XCB_VISIBILITY_NOTIFY:
        {
            xcb_visibility_notify_event_t* visibilityEvent = reinterpret_cast<xcb_visibility_notify_event_t*>(event);
            uint8_t visibilityState = visibilityEvent->state;

            if ((visibilityState >= VISIBILITY_UNOBSCURED) && (visibilityState < VISIBILITY_LAST))
            {
                VisibilityState eState = (VisibilityState)visibilityState;
                KLOG_INFO() << "visibility state changed:"
                            << visibilityEvent->window
                            << "->" << eState;
                emit visibilityStateChanged(visibilityEvent->window,eState);
            }
            else
            {
                KLOG_WARNING() << "unknow visibility state:" << visibilityEvent->window << visibilityState;
            }
            break;
        }
        default:
            break;
        }

        ::free(event);
        count++;
    }

    if(count)
        xcb_flush(m_xcbConnection);
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