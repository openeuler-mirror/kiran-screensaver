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

#ifndef KIRAN_SCREENSAVER_SRC_KS_IDLE_WATCHER_H_
#define KIRAN_SCREENSAVER_SRC_KS_IDLE_WATCHER_H_

#include <QObject>

/**
 * @brief 空闲监控接口定义
 */
namespace Kiran
{
namespace ScreenSaver
{
class IdleWatcher : public QObject
{
    Q_OBJECT
public:
    explicit IdleWatcher(QObject* parent = nullptr) : QObject(parent){};
    virtual ~IdleWatcher() = default;

public:
    // 初始化
    virtual bool init() = 0;

    // 空闲检测开启状态
    virtual bool getIdleDetectionActive() = 0;
    virtual bool setIdleDetectionActive(bool idleDetectionActive) = 0;

    // 启用状态，是否被禁用
    virtual bool getEnabled() = 0;

    // 是否启用状态
    virtual bool setEnabled(bool enabled) = 0;

signals:
    /**
     * 空闲状态改变信号
     * 由于需要槽函数改变信号发出的handled值表明是否处理完成，
     * 所以需指定Qt信号和槽函数的连接方式为Qt::DirectConnection,发出信号时直接调用槽函数。
     * \param idle      是否空闲
     * \param handled   标志槽函数是否处理完成
     */
    void idleChanged(bool idle, bool& handled);

    /**
     * 空闲状态通知信号,使用Qt::DirectConnection进行连接，直接调用槽函数进行淡出操作
     * 由于需要槽函数改变信号发出的handled值表明是否处理完成，
     * 所以需指定Qt信号和槽函数的连接方式为Qt::DirectConnection,发出信号时直接调用槽函数。
     * \param isEffect  是否有效，标志是否是处于空闲预告阶段
     * \param handled   标志槽函数是否处理完成
     */
    void idleNoticeChanged(bool isEffect, bool& handled);
};
}  // namespace ScreenSaver
}  // namespace Kiran

#endif  //KIRAN_SCREENSAVER_SRC_KS_IDLE_WATCHER_H_
