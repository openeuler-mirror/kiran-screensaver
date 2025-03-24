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
#include "classical-screensaver.h"
#include <QDateTime>
#include "float-label.h"
#include "qt5-log-i.h"
#include "ui_classical-screensaver.h"

namespace Kiran
{
namespace ScreenSaver
{
ClassicalScreensaver::ClassicalScreensaver(bool enableAnimation, QWidget *parent)
    : ScreensaverBase(enableAnimation, parent),
      ui(new Ui::ClassicalScreensaver)
{
    ui->setupUi(this);
    init();
}

ClassicalScreensaver::~ClassicalScreensaver()
{
    delete ui;
}

void ClassicalScreensaver::init()
{
    m_floatingLabel = new FloatLabel(this);
    m_floatingLabel->setAnchor(0.5, 0.93);
    m_floatingLabel->setFloatingParameter(FloatLabel::DIRECTION_UP, true, 2600, 24);
    m_floatingLabel->setText(tr("Click to unlock"));
    m_floatingLabel->setPixmap(QPixmap(":/kiran-screensaver/images/arrow.svg"), QSize(16, 16));
    // 减少CPU占用
    // m_floatingLabel->start();

    connect(this, &ScreensaverBase::updateTime, this, &ClassicalScreensaver::processUpdateTime);
    startUpdateTimeDateTimer();
}

void ClassicalScreensaver::processUpdateTime()
{
    auto dateTime = QDateTime::currentDateTime();
    QString time = dateTime.toString("hh:mm");
    QString date = dateTime.toString(tr("MM-dd dddd"));
    ui->label_time->setText(time);
    ui->label_date->setText(date);
}

}  // namespace ScreenSaver
}  // namespace Kiran