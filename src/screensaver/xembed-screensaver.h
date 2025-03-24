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

class QProcess;
class QWindow;
namespace Kiran
{
namespace ScreenSaver
{
namespace Ui
{
class XEmbedScreensaver;
}

class XEmbedScreensaver : public ScreensaverBase
{
    Q_OBJECT

public:
    explicit XEmbedScreensaver(const QString& name,
                               bool enableAnimation,
                               QWidget* parent = nullptr);
    ~XEmbedScreensaver();

    static bool isSupported(const QString& name);

private:
    void init();
    static QString getEmbedProcessPath(const QString& themeName);
    void launchEmbedProcess();

private:
    Ui::XEmbedScreensaver* ui;
    // 内部主题名: xembed-pacman
    QString m_themeName;
    // xscreensaver主题名： pacman
    QString m_screensaverName;
    // xscreensaver主题进程路径
    QString m_embedProcessPath;
    QProcess* m_embedProcess = nullptr;
    QWindow* m_foreignWindow = nullptr;
    QWidget* m_embedContainer = nullptr;
};
}  // namespace ScreenSaver
}  // namespace Kiran