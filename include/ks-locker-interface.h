//
// Created by lxh on 2021/8/27.
//

#ifndef KIRAN_SCREENSAVER_INCLUDE_KS_LOCKER_INTERFACE_H_
#define KIRAN_SCREENSAVER_INCLUDE_KS_LOCKER_INTERFACE_H_

class KSLockerInterface
{
    virtual void show(bool hasAnimation, int ms) = 0;
    virtual void hide(bool hasAnimation, int ms) = 0;
};

#endif  //KIRAN_SCREENSAVER_INCLUDE_KS_LOCKER_INTERFACE_H_
