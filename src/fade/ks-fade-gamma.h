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


#ifndef KIRAN_SCREENSAVER_SRC_FADE_KS_FADE_GAMMA_H_
#define KIRAN_SCREENSAVER_SRC_FADE_KS_FADE_GAMMA_H_

#include <QtGlobal>
#include "ks-fade-interface.h"

/**
 * @brief: 通过XGamma插件实现屏幕淡出相关功能实现
 * @note:  暂未实现
 */
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
