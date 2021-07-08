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
 
#ifndef KIRAN_SCREENSAVER_SRC_VIEW_KS_WINDOW_H_
#define KIRAN_SCREENSAVER_SRC_VIEW_KS_WINDOW_H_

#include <QWidget>

class QProcess;
class KSWindow: public QWidget
{
    Q_OBJECT
public:
    explicit KSWindow(QScreen* screen = nullptr);
    ~KSWindow() override;

    void setScreen(QScreen* screen);
    void showScreenSaver(const QString& path);

private slots:
    void handleScreenGeometryChanged(const QRect &geometry);

private:
    QScreen* m_screen = nullptr;
    QProcess* m_process = nullptr;
    QWindow* m_screensaverWindow = nullptr;
    QWidget* m_screensaverWidget = nullptr;
};

#endif  //KIRAN_SCREENSAVER_SRC_VIEW_KS_WINDOW_H_
