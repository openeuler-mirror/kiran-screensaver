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
#include "screensaver-factory.h"
#include "classical-screensaver.h"
#include "qt5-log-i.h"
#include "xembed-screensaver.h"

namespace Kiran
{
namespace ScreenSaver
{
ScreensaverBase* ScreensaverFactory::createScreensaver(const QString& themeName, bool enableAnimation, QWidget* parent)
{
    ScreensaverBase* screensaver = nullptr;
    
    if (XEmbedScreensaver::isSupported(themeName))
    {
        KLOG_INFO() << "create XEmbedScreensaver of" << themeName;
        screensaver = new XEmbedScreensaver(themeName, enableAnimation, parent);
    }

    if (!screensaver)
    {
        KLOG_INFO() << "create ClassicalScreensaver of" << themeName;
        screensaver = new ClassicalScreensaver(enableAnimation, parent);
    }
    return screensaver;
}
}  // namespace ScreenSaver
}  // namespace Kiran
