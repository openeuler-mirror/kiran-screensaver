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

#ifndef KIRAN_SCREENSAVER_SRC_SCREENSAVER_FLOAT_LABEL_H_
#define KIRAN_SCREENSAVER_SRC_SCREENSAVER_FLOAT_LABEL_H_

#include "float-widget.h"

class QLabel;
class FloatLabel : public FloatWidget
{
    Q_OBJECT
public:
    explicit FloatLabel(QWidget* parent = nullptr);
    ~FloatLabel();

    void setText(const QString& text);
    void setPixmap(const QPixmap& pixmap, const QSize& size);

private:
    QLabel* m_labelPixmap  = nullptr;
    QLabel* m_labelText = nullptr;
};

#endif  //KIRAN_SCREENSAVER_SRC_SCREENSAVER_FLOAT_LABEL_H_
