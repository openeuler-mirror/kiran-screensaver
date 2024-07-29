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
#pragma once
#include <QObject>
#include <QSocketNotifier>
#include <QWidget>
#include <QX11Info>
#include <QSet>

struct xcb_connection_t;
struct xcb_ge_generic_event_t;
class QSocketNotifier;
namespace Kiran
{
namespace ScreenSaver
{
/**
 * NOTE:
 * 保证 kiran-screensaver 窗口被置顶
 * 1.在不存在混成器，XServer会发出VisibilityNotify事件通知窗口可见状态
 * 2.订阅MapNotify,ConfigureNotify,窗口堆叠有更改时，重新置顶
*/
class VisibilityMonitor : public QObject
{
    Q_OBJECT
public:
    enum VisibilityState
    {
        VISIBILITY_UNOBSCURED = 0,          // 完全可见
        VISIBILITY_PARTIALLY_OBSCURED = 1,  // 部分可见
        VISIBILITY_FULLY_OBSCURED = 2,      // 完全不可见
        VISIBILITY_LAST = 3
    };
    Q_ENUM(VisibilityState);

    static VisibilityMonitor* instance();
    ~VisibilityMonitor() override;

    void monitor(WId wid);
    void unmonitor(WId wid);

signals:
    void visibilityStateChanged(WId wid,VisibilityState state);
    void restackedNeedRaise();

private:
    explicit VisibilityMonitor(QObject* parent = nullptr);
    void init();
    bool isInternal(WId window);
    void selectSubstructureNotify();
    void unselectSubstructureNotify();
    void handleXcbEvent();
    void onVisibilityNotify(xcb_generic_event_t* event);
    void onMapNotify(xcb_generic_event_t* event);
    void onConfigureNotify(xcb_generic_event_t* event);

private slots:
#if (QT_VERSION >= QT_VERSION_CHECK(5, 15, 0))
    void onXcbSocketNotifierActivated(QSocketDescriptor socket, QSocketNotifier::Type activationEvent);
#else
    void onXcbSocketNotifierActivated(int socket);
#endif

private:
    bool m_inited = false;
    xcb_connection_t* m_xcbConnection = nullptr;
    QSocketNotifier* m_xcbSocketNotifier = nullptr;
    QSet<WId> m_windows;
};
}  // namespace ScreenSaver
}  // namespace Kiran