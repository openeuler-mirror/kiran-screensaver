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
#include "xembed-screensaver.h"
#include <QDateTime>
#include <QFileInfo>
#include <QProcess>
#include <QTimer>
#include <QWindow>
#include "qt5-log-i.h"
#include "screensaver-base.h"
#include "ui_xembed-screensaver.h"

#define XEMBED_SCREENSAVER_PREFIX "xembed-"
#define XSCREENSAVER_PATH "/usr/libexec/xscreensaver/"

// 特殊的主题参数，同时也是内置推荐的主题
// antspotlight 开销过大
static QMap<QString, QStringList> speciallyThemeArgs = {
    {"atlantis", {"--count", "2", "--size", "100"}},
    {"binaryhorizon", {"--particles-number", "1000", "--growth-delay", "58000"}},
    {"epicycle",{}},
    {"rubik",{"--count","20"}}
};

namespace Kiran
{
namespace ScreenSaver
{
XEmbedScreensaver::XEmbedScreensaver(const QString& name, bool enableAnimation, QWidget* parent)
    : ScreensaverBase(false, parent), // xembed xscreensaver主题不太适合这个过渡动画,保持禁用
      ui(new Ui::XEmbedScreensaver),
      m_themeName(name)

{
    ui->setupUi(this);
    init();
}

XEmbedScreensaver::~XEmbedScreensaver()
{
    if (m_embedProcess && m_embedProcess->state() == QProcess::Running)
    {
        m_embedProcess->kill();
    }
    delete ui;
}

bool XEmbedScreensaver::isSupported(const QString& name)
{
    if (!name.startsWith(XEMBED_SCREENSAVER_PREFIX))
    {
        return false;
    }

    auto embedProgramPath = getEmbedProcessPath(name);
    if (!QFileInfo::exists(embedProgramPath))
    {
        KLOG_INFO() << "XEmbed screensaver program not found: " << embedProgramPath;
        return false;
    }

    return true;
}

void XEmbedScreensaver::init()
{
    m_embedProcessPath = getEmbedProcessPath(m_themeName);

    QFileInfo embedFileInfo(m_embedProcessPath);
    m_screensaverName = embedFileInfo.baseName();

    m_foreignWindow = new QWindow();
    m_foreignWindow->setFlags(Qt::ForeignWindow);
    m_foreignWindow->create();

    m_embedContainer = QWidget::createWindowContainer(m_foreignWindow, this);
    ui->layout_content->addWidget(m_embedContainer);

    // 延时启动XScreensaver屏保
    QTimer::singleShot(0,this,&XEmbedScreensaver::launchEmbedProcess);
}

QString XEmbedScreensaver::getEmbedProcessPath(const QString& themeName)
{
    // xembed-pacman
    if (!themeName.startsWith(XEMBED_SCREENSAVER_PREFIX))
    {
        return QString();
    }

    // pacman
    auto xscreensaverName = themeName.mid(QString(XEMBED_SCREENSAVER_PREFIX).length());

    // XSCREENSAVER_PATH/pacman
    auto embedProgramPath = QString("%1/%2").arg(XSCREENSAVER_PATH, xscreensaverName);
    return embedProgramPath;
}

void XEmbedScreensaver::launchEmbedProcess()
{
    QString windowIDArg = QString("0x%1").arg(QString::number(m_foreignWindow->winId(), 16));

    QStringList launchArgs{
        "--window-id", windowIDArg,
        "--noinstall",
        // "--fps"
    };

    if (speciallyThemeArgs.contains(m_screensaverName))
    {
        launchArgs << speciallyThemeArgs[m_screensaverName];
    }

    KLOG_INFO() << "Starting embed screensaver process" << m_embedProcessPath << launchArgs;
    m_embedProcess = new QProcess(this);
    connect(m_embedProcess, &QProcess::errorOccurred, this, [this](QProcess::ProcessError error)
            { KLOG_INFO() << "embed process error: " << error << m_embedProcess->errorString() << m_embedProcess->readAllStandardError(); });
    connect(m_embedProcess, &QProcess::stateChanged, this, [this](QProcess::ProcessState state)
            { KLOG_INFO() << "embed process state: " << state; });
    connect(m_embedProcess, &QProcess::readyReadStandardError, this, [this]()
            { KLOG_INFO() << "embed process has error:" << m_embedProcess->readAllStandardError(); });
    m_embedProcess->start(m_embedProcessPath, launchArgs);
}

}  // namespace ScreenSaver
}  // namespace Kiran
