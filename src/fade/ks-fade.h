/**
 * @file   ks-fade.h
 * @biref  屏幕淡出功能实现类
 * @author liuxinhao <liuxinhao@kylinos.com.cn>
 * @copyright (c) 2021 KylinSec. All rights reserved.
 */

#ifndef KIRAN_SCREENSAVER_SRC_KS_FADE_H_
#define KIRAN_SCREENSAVER_SRC_KS_FADE_H_

#include <QObject>
#include "ks-fade-interface.h"

class KSFade : public QObject
{
    Q_OBJECT
public:
    explicit KSFade(QObject* parent = nullptr);
    ~KSFade() override;

public:
    ///获取是否正在淡出
    bool getActive() const;
    ///开始淡出
    bool startAsync();
    ///重置复位
    void reset();

signals:
    ///淡出完成信号
    void faded();

protected:
    void timerEvent(QTimerEvent* event) override;

private:
    ///停止淡出
    void stop();
    ///处理淡出定时器超时时间
    bool handleFadeTimeout();

    void fadeFinish();

private:
    KSFadeInterface* m_fadeInterface = nullptr;
    bool m_active = false;
    int m_fadeTimerID = 0;
    qreal m_currentAlpha = 1.0;
    qreal m_fadeStepAlpha = 0.0;
};

#endif  //KIRAN_SCREENSAVER_SRC_KS_FADE_H_
