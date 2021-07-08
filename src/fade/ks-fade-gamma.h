//
// Created by lxh on 2021/7/2.
//

#ifndef KIRAN_SCREENSAVER_SRC_FADE_KS_FADE_GAMMA_H_
#define KIRAN_SCREENSAVER_SRC_FADE_KS_FADE_GAMMA_H_

#include <QtGlobal>
#include "ks-fade-interface.h"

class KSFadeGamma : public KSFadeInterface
{
public:
    KSFadeGamma();
    ~KSFadeGamma();

public:
    static bool checkForSupport();
    bool setup() override;
    bool setAlphaGamma(double alpha) override;
    void finish() override;
};

#endif  //KIRAN_SCREENSAVER_SRC_FADE_KS_FADE_GAMMA_H_
