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
#include <QX11Info>
#include <QScopedPointer>
#include <qt5-log-i.h>
#include <memory> //std::unique_ptr
#include <xcb/xcb.h>

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