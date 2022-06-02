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

#ifndef KIRAN_SCREENSAVER_INCLUDE_PLUGIN_INTERFACE_H_
#define KIRAN_SCREENSAVER_INCLUDE_PLUGIN_INTERFACE_H_

#include <QObject>
#include "interface.h"
#include "locker-interface.h"

namespace Kiran
{
namespace ScreenSaver
{
class PluginInterface
{
public:
    virtual ~PluginInterface() = default;

    virtual int init(Interface* ksInterface) = 0;
    virtual void uninit() = 0;

    // 获取解锁窗口
    virtual LockerInterface* createLocker() = 0;

    // 获取屏保窗口
    //QWidget* createScreenSaverWidget()
};
}
}
#define PluginInterface_iid "com.kylinsec.Kiran.ScreenSaver.PluginInterface/1.0"
Q_DECLARE_INTERFACE(Kiran::ScreenSaver::PluginInterface,PluginInterface_iid)
#endif  //KIRAN_SCREENSAVER_INCLUDE_PLUGIN_INTERFACE_H_
