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

#ifndef KIRAN_SCREENSAVER_SRC_FADE_FADE_INTERFACE_H_
#define KIRAN_SCREENSAVER_SRC_FADE_FADE_INTERFACE_H_

/**
 * @interface KSFade淡出接口,相关具体实现需继承该接口进行实现
 */

namespace Kiran
{
namespace ScreenSaver
{
class FadeInterface
{
public:
    FadeInterface(){};
    virtual ~FadeInterface() = default;

    ///保存屏幕的gamma信息
    virtual bool setup() = 0;
    ///恢复屏幕的gamma信息
    virtual void finish() = 0;
    ///中途更新屏幕gamma信息
    virtual bool setAlphaGamma(double alpha) = 0;
};
}  // namespace ScreenSaver
}  // namespace Kiran

#endif  //KIRAN_SCREENSAVER_SRC_FADE_FADE_INTERFACE_H_
