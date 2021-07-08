//
// Created by lxh on 2021/7/2.
//

#ifndef KIRAN_SCREENSAVER_SRC_FADE_KS_FADE_INTERFACE_H_
#define KIRAN_SCREENSAVER_SRC_FADE_KS_FADE_INTERFACE_H_

class KSFadeInterface
{
public:
    KSFadeInterface(){};
    virtual ~KSFadeInterface() = default;

    ///保存屏幕的gamma信息
    virtual bool setup() = 0;
    ///恢复屏幕的gamma信息
    virtual void finish() = 0;
    ///中途更新屏幕gamma信息
    virtual bool setAlphaGamma(double alpha) = 0;

};
#endif  //KIRAN_SCREENSAVER_SRC_FADE_KS_FADE_INTERFACE_H_
