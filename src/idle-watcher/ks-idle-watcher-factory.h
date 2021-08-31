//
// Created by lxh on 2021/8/12.
//

#ifndef KIRAN_SCREENSAVER_SRC_IDLE_WATCHER_KS_IDLE_WATCHER_FACTORY_H_
#define KIRAN_SCREENSAVER_SRC_IDLE_WATCHER_KS_IDLE_WATCHER_FACTORY_H_

#include "ks-idle-watcher.h"

class KSIdleWatcherFactory
{
public:
    ~KSIdleWatcherFactory() = default;
private:
    KSIdleWatcherFactory() = default;
public:
    static KSIdleWatcher* createIdleWatcher();
};

#endif  //KIRAN_SCREENSAVER_SRC_IDLE_WATCHER_KS_IDLE_WATCHER_FACTORY_H_
