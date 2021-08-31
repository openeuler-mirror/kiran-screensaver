//
// Created by lxh on 2021/8/12.
//
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
