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

#ifndef __KS_PLUGIN_I_H__
#define __KS_PLUGIN_I_H__

#include <QObject>
#include "ks-locker-interface.h"
#include "ks-interface.h"

class KSPluginInterface
{
public:
    virtual ~KSPluginInterface() = default;

    virtual int init(KSInterface* ksInterface) = 0;
    virtual void uninit() = 0;

    // 获取解锁窗口
    virtual KSLockerInterface* createLocker() = 0;

    // 获取屏保窗口
    //QWidget* createScreenSaverWidget()
};

#define KSPluginInterface_iid "com.kylinsec.Kiran.KSPluginInterface/1.0"
Q_DECLARE_INTERFACE(KSPluginInterface,KSPluginInterface_iid)

#endif  //__KS_PLUGIN_I_H__
