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