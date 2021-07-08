/**
  * @Copyright (C) 2020 ~ 2021 KylinSec Co., Ltd. 
  *
  * Author:     liuxinhao <liuxinhao@kylinos.com.cn>
  *
  * This program is free software; you can redistribute it and/or modify
  * it under the terms of the GNU General Public License as published by
  * the Free Software Foundation; either version 3 of the License, or
  * (at your option) any later version.
  *
  * This program is distributed in the hope that it will be useful,
  * but WITHOUT ANY WARRANTY; without even the implied warranty of
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  * GNU General Public License for more details.
  *
  * You should have received a copy of the GNU General Public License
  * along with this program; If not, see <http: //www.gnu.org/licenses/>. 
  */

#include "ks-window.h"

#include <QProcess>
#include <QWindow>
#include <qt5-log-i.h>
#include <QBoxLayout>
#include <QScreen>

KSWindow::KSWindow(QScreen *screen)
    :QWidget(nullptr)
{
    setWindowFlag(Qt::X11BypassWindowManagerHint);

    m_screensaverWindow = new QWindow;
    m_screensaverWindow->create();

    m_screensaverWidget = createWindowContainer(m_screensaverWindow,this);
    auto layout = new QHBoxLayout(this);
    layout->setMargin(0);
    this->layout()->addWidget(m_screensaverWidget);

    m_process = new QProcess(this);
}

KSWindow::~KSWindow()
{
    delete m_screensaverWindow;
    if(m_process->state() != QProcess::NotRunning)
    {
        m_process->terminate();
        m_process->waitForFinished();
    }
}

void KSWindow::showScreenSaver(const QString &path)
{
    if(m_process->state() != QProcess::NotRunning)
    {
        //TODO:
    }
    m_process->start(path,QStringList() << "-window-id" << QString::number(m_screensaverWindow->winId()),QIODevice::ReadOnly);
}

void KSWindow::setScreen(QScreen *screen)
{
    if(m_screen == screen)
        return;

    if(m_screen)
    {
        disconnect(m_screen,&QScreen::geometryChanged,
                   this,&KSWindow::handleScreenGeometryChanged);
    }

    if(screen)
    {
        connect(screen,&QScreen::geometryChanged,
                this,&KSWindow::handleScreenGeometryChanged);
    }

    m_screen = screen;
    if(m_screen)
        handleScreenGeometryChanged(m_screen->geometry());
}

void KSWindow::handleScreenGeometryChanged(const QRect &geometry)
{
    setGeometry(geometry);
}
