//
// Created by lxh on 2021/9/28.
//

#include "float-label.h"

#include <QVBoxLayout>
#include <QLabel>

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
    m_labelText->setStyleSheet("QLabel{font-family:Noto Sans CJK SC Light; font-size:14px;}");
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
