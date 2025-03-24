/**
 * Copyright (c) 2020 ~ 2025 KylinSec Co., Ltd.
 * kiran-screensaver is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 *
 * Author:     liuxinhao <liuxinhao@kylinsec.com.cn>
 */
#pragma once
#include "screensaver-base.h"

class QWidget;

namespace Kiran
{
namespace ScreenSaver
{
class ScreensaverFactory
{
public:
    ScreensaverFactory() = delete;
    static Kiran::ScreenSaver::ScreensaverBase* createScreensaver(const QString& themeName,bool enableAnimation, QWidget* parent = nullptr);
};
}  // namespace ScreenSaver
}  // namespace Kiran