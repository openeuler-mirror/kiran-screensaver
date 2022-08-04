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
#include "float-label.h"

#include <QVBoxLayout>
#include <QLabel>

using namespace Kiran::ScreenSaver;

FloatLabel::FloatLabel(QWidget *parent)
    : FloatWidget(parent)
{
    auto layout = new QVBoxLayout(this);
    layout->setMargin(0);
    layout->setSpacing(8);

    m_labelPixmap = new QLabel(this);
    m_labelPixmap->setAlignment(Qt::AlignCenter);
    layout->addWidget(m_labelPixmap,Qt::AlignHCenter);

    m_labelText = new QLabel(this);
    m_labelText->setStyleSheet("QLabel{font-family:Noto Sans CJK SC Light; font-size:14px;color:#fefefe;}");
    m_labelText->setAlignment(Qt::AlignCenter);
    layout->addWidget(m_labelText,Qt::AlignHCenter);

    adjustSize();
}

FloatLabel::~FloatLabel()
{
}

void FloatLabel::setText(const QString &text)
{
    m_labelText->setText(text);
    adjustSize();
}

void FloatLabel::setPixmap(const QPixmap& pixmap, const QSize& size)
{
    m_labelPixmap->setPixmap(pixmap);
    m_labelPixmap->setFixedSize(size);
    layout()->setAlignment(m_labelPixmap,Qt::AlignHCenter);
    adjustSize();
}
