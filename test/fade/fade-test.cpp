//
// Created by lxh on 2021/7/7.
//

#include "fade-test.h"
#include "ks-fade.h"

#include <qt5-log-i.h>
#include <QApplication>

FadeTest::FadeTest(QObject *parent) : QObject(parent)
{
    m_fade = new KSFade(this);
    connect(m_fade,&KSFade::faded,this,[this](){
        KLOG_INFO() << "fade finish";
        m_fade->reset();
        qApp->quit();
    });
}

FadeTest::~FadeTest()
{
}

bool FadeTest::start()
{
    return m_fade->startAsync();
}
