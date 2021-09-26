//
// Created by lxh on 2021/9/17.
//

#ifndef KIRAN_SCREENSAVER_INCLUDE_KS_LOCKER_INTERFACE_H_
#define KIRAN_SCREENSAVER_INCLUDE_KS_LOCKER_INTERFACE_H_

class QWidget;
class KSLockerInterface
{
public:
    KSLockerInterface() = default;
    virtual ~KSLockerInterface() = default;
public:
    //获取QWidget类型指针
    virtual QWidget* get_widget_ptr() = 0;

    //设置是否开启动画
    virtual void setAnimationEnabled(bool enabled) = 0;
    virtual void setAnimationDuration(int fadeInMs,int fadeOutMs) = 0;

    //淡入淡出选项
    virtual bool fadeVisible() = 0;
    virtual bool fadeIn() = 0;
    virtual bool fadeOut() = 0;
};

#endif  //KIRAN_SCREENSAVER_INCLUDE_KS_LOCKER_INTERFACE_H_
