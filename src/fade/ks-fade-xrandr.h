//
// Created by lxh on 2021/7/2.
//

#ifndef KIRAN_SCREENSAVER_SRC_FADE_KS_FADE_XRANDR_H_
#define KIRAN_SCREENSAVER_SRC_FADE_KS_FADE_XRANDR_H_

#include <QtGlobal>

#include "ks-fade-interface.h"

struct KSFadeXrandrPrivate;

/**
 * @brief 基于X11 xrandr插件接口实现的屏幕淡出具体实现
 */
class KSFadeXrandr : public KSFadeInterface
{
public:
    KSFadeXrandr();
    ~KSFadeXrandr();

public:
    static bool checkForSupport();
    bool setup() override;
    bool setAlphaGamma(double alpha) override;
    void finish() override;

private:
    void cleanup();

private:
    KSFadeXrandrPrivate* d_ptr = nullptr;
};

#endif  //KIRAN_SCREENSAVER_SRC_FADE_KS_FADE_XRANDR_H_
