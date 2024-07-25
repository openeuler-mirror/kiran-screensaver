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

struct xcb_connection_t;
class QSocketNotifier;
namespace Kiran
{
namespace ScreenSaver
{
/**
 * NOTE:
 * 请注意，VisibilityNotify事件只有在Marco关闭混合的情况下，XServer才会发送
 * 该类只是一个保护措施，Marco并不管理屏保窗口，XServer也只会保证XScreenSaver窗口置顶
 * 存在可能Marco同步窗口堆叠至XServer之中时，锚定窗口位于kiran-screensaver窗口之上
 * 导致Marco窗口堆叠Stack整体窗口位于kiran-screensaver窗口之上
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

    void monitorWindow(WId wid);

signals:
    void visibilityStateChanged(WId wid,VisibilityState state);

private:
    explicit VisibilityMonitor(QObject* parent = nullptr);
    void init();
    void handleXcbEvent();

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
};
}  // namespace ScreenSaver
}  // namespace Kiran