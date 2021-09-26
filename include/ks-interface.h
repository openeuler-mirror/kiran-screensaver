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

#ifndef KIRAN_SCREENSAVER_INCLUDE_KS_INTERFACE_H_
#define KIRAN_SCREENSAVER_INCLUDE_KS_INTERFACE_H_

class KSInterface
{
public:
    ///解锁框插件调用该接口通知kiran-screensaver已通过认证，认证成功,并进行解锁
    virtual void authenticationPassed() = 0;
};

#endif  //KIRAN_SCREENSAVER_INCLUDE_KS_INTERFACE_H_
