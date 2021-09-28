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

#include "ks-idle-watcher-factory.h"
#include <qt5-log-i.h>
#include "ks-idle-watcher-mate.h"

KSIdleWatcher* KSIdleWatcherFactory::createIdleWatcher()
{
    KSIdleWatcher* res = nullptr;

    QString desktopEnv = qgetenv("XDG_CURRENT_DESKTOP");
    if(desktopEnv.compare("MATE",Qt::CaseInsensitive) == 0)
    {
        res = new KSIdleWatcherMate;
    }
    else
    {
        KLOG_WARNING() << "not supported this DE <" << desktopEnv << ">";
    }

    return res;
}
