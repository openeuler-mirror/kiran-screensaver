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
#include <QCursor>
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

// 传给 xscreensaver hack 的参数必须使用单横线形式(-xxx), 不能使用双横线(--xxx):
// 5.x 的 hack 通过 Xt/Xmu 解析命令行(XtAppInitialize -> XrmParseCommand), 选项表的键都是单横线,
// 且 5.x 没有 6.x 中"把 --xxx 归一化为 -xxx"的兼容处理(见 6.x hacks/screenhack.c,
// 注释: Xt and xscreensaver predate the "--arg" convention, so convert double dashes to single.),
// 传入 --xxx 会使得参数全部无法被识别, hack 打印 Usage + "unrecognised option" 后 exit(1),
// 嵌入主题表现为黑屏。单横线形式在 5.x/6.x 上都可用, 因此此处统一使用单横线, 请勿改回双横线。

// 特殊的主题参数，同时也是内置推荐的主题
// antspotlight 开销过大
static QMap<QString, QStringList> speciallyThemeArgs = {
    {"atlantis", {"-count", "2", "-size", "100"}},
    {"binaryhorizon", {"-particles-number", "1000", "-growth-delay", "58000"}},
    {"epicycle",{}},
    {"rubik",{"-count","20"}}
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
    m_foreignWindow->setCursor(Qt::BlankCursor);

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

QStringList XEmbedScreensaver::buildLaunchArgs(const QString& screensaverName, quint64 windowId)
{
    // 统一使用单横线参数, 详见文件头部说明
    QStringList launchArgs{
        "-window-id", QString("0x%1").arg(windowId, 0, 16),
        "-noinstall",
        // "-fps"
    };

    launchArgs << speciallyThemeArgs.value(screensaverName);
    return launchArgs;
}

void XEmbedScreensaver::launchEmbedProcess()
{
    QStringList launchArgs = buildLaunchArgs(m_screensaverName, m_foreignWindow->winId());

    KLOG_INFO() << "Starting embed screensaver process" << m_embedProcessPath << launchArgs;
    m_embedProcess = new QProcess(this);
    connect(m_embedProcess, &QProcess::errorOccurred, this, [this](QProcess::ProcessError error)
            { KLOG_INFO() << "embed process error: " << error << m_embedProcess->errorString() << m_embedProcess->readAllStandardError(); });
    connect(m_embedProcess, &QProcess::stateChanged, this, [this](QProcess::ProcessState state)
            { KLOG_INFO() << "embed process state: " << state; });
    connect(m_embedProcess, &QProcess::readyReadStandardError, this, [this]()
            { KLOG_INFO() << "embed process has error:" << m_embedProcess->readAllStandardError(); });
    connect(m_embedProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, [this](int exitCode, QProcess::ExitStatus exitStatus)
            { KLOG_WARNING() << "embed process finished, exitCode:" << exitCode << "exitStatus:" << exitStatus; });
    m_embedProcess->start(m_embedProcessPath, launchArgs);
}

}  // namespace ScreenSaver
}  // namespace Kiran
