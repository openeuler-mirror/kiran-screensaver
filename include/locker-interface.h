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

#ifndef KIRAN_SCREENSAVER_INCLUDE_LOCKER_INTERFACE_H_
#define KIRAN_SCREENSAVER_INCLUDE_LOCKER_INTERFACE_H_

class QWidget;

namespace Kiran
{
namespace ScreenSaver
{
class LockerInterface
{
public:
    LockerInterface() = default;
    virtual ~LockerInterface() = default;

public:
    //获取QWidget类型指针
    virtual QWidget* get_widget_ptr() = 0;

    //设置是否开启动画
    virtual void setAnimationEnabled(bool enabled) = 0;
    virtual void setAnimationDelay(int fadeInDelay, int fadeOutDelay) = 0;
    virtual void setAnimationDuration(int fadeInMs, int fadeOutMs) = 0;

    //淡入淡出选项
    virtual bool fadeVisible() = 0;
    virtual bool fadeIn() = 0;
    virtual bool fadeOut() = 0;

    //解锁框相关配置项
    // 切换用户按钮
    virtual void setEnableSwitch(bool enable) = 0;
    virtual bool enableSwitch() = 0;
};
}  // namespace ScreenSaver
}  // namespace Kiran

#endif  //KIRAN_SCREENSAVER_INCLUDE_LOCKER_INTERFACE_H_
