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
#include "ks-interface.h"
#include "ks-locker-interface.h"
#include "ks-screensaver-interface.h"

//TODO:暂定,先完成锁屏和屏保,再考虑抽象成借口
class KSPluginInterface : public QObject
{
public:
    enum LockerOptions{
        LO_ENABLE_LOGOUT,
        LO_ENABLE_SWITCH
    };
    Q_FLAG(LockerOptions)

    virtual ~KSPluginInterface() = default;

    virtual int init() = 0;
    virtual void uninit() = 0;

    // 获取解锁窗口
    QWidget* createLockerWidget(KsLockerInterface* interface,LockerOptions options) = 0;

    // 获取屏保窗口
    QWidget* createScreenSaverWidget()
};

#define KSPluginInterface_iid "com.kylinsec.Kiran.KSPluginInterface/1.0"
Q_DECLARE_INTERFACE(KSPluginInterface,KSPluginInterface_iid)

#endif  //__KS_PLUGIN_I_H__
