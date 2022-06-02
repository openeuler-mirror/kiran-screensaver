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

#ifndef KIRAN_SCREENSAVER_TEST_FADE_FADE_TEST_H_
#define KIRAN_SCREENSAVER_TEST_FADE_FADE_TEST_H_

#include <QObject>

class KSFade;
class FadeTest : public QObject
{
    Q_OBJECT
public:
    FadeTest(QObject* parent= nullptr);
    ~FadeTest();

    bool start();

private:
    KSFade* m_fade = nullptr;
};

#endif  //KIRAN_SCREENSAVER_TEST_FADE_FADE_TEST_H_
