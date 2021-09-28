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

#ifndef KIRAN_SCREENSAVER_SRC_KS_INVISIBLE_WINDOW_H_
#define KIRAN_SCREENSAVER_SRC_KS_INVISIBLE_WINDOW_H_

#include <QWidget>

/**
 * @brief 封装的在屏幕之外的窗口(不进行显示),负责在屏幕淡出时抓取输入设备到该窗口,避免误输入
 */
class KSInvisibleWindow : public QWidget
{
    Q_OBJECT
public:
    KSInvisibleWindow(QWidget* parent= nullptr);
    ~KSInvisibleWindow();
};

#endif  //KIRAN_SCREENSAVER_SRC_KS_INVISIBLE_WINDOW_H_
