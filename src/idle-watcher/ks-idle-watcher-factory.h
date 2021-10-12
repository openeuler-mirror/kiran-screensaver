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
