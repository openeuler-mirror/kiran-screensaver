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

#ifndef KIRAN_SCREENSAVER_SRC_FADE_FADE_XRANDR_H_
#define KIRAN_SCREENSAVER_SRC_FADE_FADE_XRANDR_H_

#include <QtGlobal>
#include "fade-interface.h"

struct KSFadeXrandrPrivate;

/**
 * @brief 基于X11 xrandr插件接口实现的屏幕淡出具体实现
 */
namespace Kiran
{
namespace ScreenSaver
{
class FadeXrandr : public FadeInterface
{
public:
    FadeXrandr();
    FadeXrandr(FadeXrandr& other) = delete;
    FadeXrandr& operator=(FadeXrandr& other) = delete;
    ~FadeXrandr();
    
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
}  // namespace ScreenSaver
}  // namespace Kiran

#endif  //KIRAN_SCREENSAVER_SRC_FADE_FADE_XRANDR_H_
