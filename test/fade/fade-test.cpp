/**
 * Copyright (c) 2020 ~ 2021 KylinSec Co., Ltd.
 * kiran-screensaver is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 *
 * Author:     liuxinhao <liuxinhao@kylinos.com.cn>
 */

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
