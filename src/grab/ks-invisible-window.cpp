//
// Created by lxh on 2021/7/7.
//

#include "ks-invisible-window.h"

KSInvisibleWindow::KSInvisibleWindow(QWidget *parent)
    :QWidget(parent)
{
    setWindowFlag(Qt::BypassWindowManagerHint);
    setGeometry(QRect(-100,-100,10,10));
}

KSInvisibleWindow::~KSInvisibleWindow()
{
}