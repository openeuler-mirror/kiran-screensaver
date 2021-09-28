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

#ifndef KIRAN_SCREENSAVER_SRC_KS_FADE_H_
#define KIRAN_SCREENSAVER_SRC_KS_FADE_H_

#include <QObject>
#include "ks-fade-interface.h"

/**
 * @brief: 提供屏幕淡出相关封装的接口
 */
class KSFade : public QObject
{
    Q_OBJECT
public:
    explicit KSFade(QObject* parent = nullptr);
    ~KSFade() override;

public:
    // 获取是否正在淡出
    bool getActive() const;
    // 开始淡出
    bool startAsync();
    // 重置复位
    void reset();

signals:
    // 淡出完成信号
    void faded();

protected:
    void timerEvent(QTimerEvent* event) override;

private:
    // 停止淡出
    void stop();
    // 淡出完成，发送淡出完成信号
    void fadeFinish();

    // 处理淡出定时器超时时间
    bool handleFadeTimeout();


private:
    // 淡出接口
    KSFadeInterface* m_fadeInterface = nullptr;
    // 是否正在淡出
    bool m_active = false;
    // 淡出定时器ID
    int m_fadeTimerID = 0;
    // 当前淡出的alpha值
    qreal m_currentAlpha = 1.0;
    // 每一步减少的alpha值
    qreal m_fadeStepAlpha = 0.0;
};

#endif  //KIRAN_SCREENSAVER_SRC_KS_FADE_H_
