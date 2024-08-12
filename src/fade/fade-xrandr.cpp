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

#include "fade-xrandr.h"
#include "xcb-randr-utils.h"

#include <qt5-log-i.h>
#include <xcb/xcb.h>

using namespace Xcb::Randr;

struct KSGammInfo
{
    int size;
    uint16_t *r;
    uint16_t *g;
    uint16_t *b;
};

struct KSFadeXrandrPrivate
{
    QMap<xcb_randr_crtc_t,KSGammInfo*> crtcGammInfo;
};

using namespace Kiran::ScreenSaver;
FadeXrandr::FadeXrandr()
    :d_ptr(new KSFadeXrandrPrivate)
{

}

FadeXrandr::~FadeXrandr()
{
    cleanup();
    delete d_ptr;
}

bool FadeXrandr::checkForSupport()
{
    bool support = false;
    xcb_randr_query_version_cookie_t versionCookie;

    xcb_connection_t* conn = xcb_connect(nullptr, nullptr);
    if(!conn)
    {
        KLOG_WARNING() << "can't connect xserver!";
        return false;
    }

    versionCookie = xcb_randr_query_version(conn,5,5);
    QScopedPointer<xcb_randr_query_version_reply_t,QScopedPointerPodDeleter>
        versionReply(xcb_randr_query_version_reply(conn,versionCookie, nullptr));
    xcb_flush(conn);

    if( versionReply != nullptr )
    {
        support = true;
    }

    xcb_disconnect(conn);
    return support;
}

bool FadeXrandr::setup()
{
    cleanup();

    ScreenResources res(QX11Info::appRootWindow());
    if(res.isNull())
    {
        KLOG_ERROR() << "can't get screen resources!";
        return false;
    }

    for(int i=0;i<res->num_crtcs;i++)
    {
        auto crtc = res.crtcs()[i];

        CrtcInfo crtcInfo(crtc,QX11Info::getTimestamp());
        CrtcGamma crtcGamma(crtc);

        if(crtcGamma.isNull() || crtcInfo.isNull())
        {
            KLOG_WARNING() << "can't get crtcGamma info or crtc info" << crtc;
            continue;
        }

        if(crtcInfo.rect().isNull())
        {
            KLOG_DEBUG() << "crtc" << crtc << "rect is null,ignore it";
            continue;
        }

        KLOG_DEBUG() << "crtc info:" << crtcInfo.rect();

        KSGammInfo* gammInfo = new KSGammInfo;
        gammInfo->size = crtcGamma->size;
        gammInfo->r = new uint16_t[gammInfo->size];
        memcpy(gammInfo->r,crtcGamma.red(),gammInfo->size*sizeof(uint16_t));
        gammInfo->g = new uint16_t[gammInfo->size];
        memcpy(gammInfo->g,crtcGamma.green(),gammInfo->size*sizeof(uint16_t));
        gammInfo->b = new uint16_t[gammInfo->size];
        memcpy(gammInfo->b,crtcGamma.blue(),gammInfo->size*sizeof(uint16_t));
        d_ptr->crtcGammInfo[crtc] = gammInfo;
    }

    return true;
}

bool FadeXrandr::setAlphaGamma(double alpha)
{
    if(d_ptr->crtcGammInfo.isEmpty())
    {
        return false;
    }

    if(alpha<0)
    {
        alpha = 0;
    }
    if(alpha>1)
    {
        alpha = 1;
    }

    for(auto iter=d_ptr->crtcGammInfo.begin();
         iter!=d_ptr->crtcGammInfo.end();
         iter++)
    {
        QScopedArrayPointer<uint16_t> r(new uint16_t[iter.value()->size]);
        QScopedArrayPointer<uint16_t> g(new uint16_t[iter.value()->size]);
        QScopedArrayPointer<uint16_t> b(new uint16_t[iter.value()->size]);
        for(int i=0;i<iter.value()->size;i++)
        {
            r[i] = iter.value()->r[i]*alpha;
            g[i] = iter.value()->g[i]*alpha;
            b[i] = iter.value()->b[i]*alpha;
        }
        xcb_randr_set_crtc_gamma(Xcb::default_connection(),iter.key(),iter.value()->size,r.data(),g.data(),b.data());
    }

    return true;
}

void FadeXrandr::finish()
{
    cleanup();
}

void FadeXrandr::cleanup()
{
    for(auto iter=d_ptr->crtcGammInfo.begin();
         iter!=d_ptr->crtcGammInfo.end();
         )
    {
        if(iter.value()->r)
        {
            delete iter.value()->r;
        }
        if(iter.value()->g)
        {
            delete iter.value()->g;
        }
        if(iter.value()->b)
        {
            delete iter.value()->b;
        }
        delete iter.value();
        d_ptr->crtcGammInfo.erase(iter++);
    }
}
