//
// Created by lxh on 2021/7/7.
//
#include <qt5-log-i.h>
#include <QApplication>
#include "fade-test.h"

int main(int argc,char* argv[])
{
    klog_qt5_init("","kylinsec-session","kiran-screensaver","fade-test");
    QApplication app(argc,argv);
    FadeTest fadeTest;
    fadeTest.start();
    return app.exec();
}