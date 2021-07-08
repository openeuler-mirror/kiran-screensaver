/**
 * @file   ks-idle-watcher.h
 * @biref  监听Session DBus空闲状态抽象类
 * @author liuxinhao <liuxinhao@kylinos.com.cn>
 * @copyright (c) 2021 KylinSec. All rights reserved. 
 */
#ifndef KIRAN_SCREENSAVER_SRC_KS_IDLE_WATCHER_H_
#define KIRAN_SCREENSAVER_SRC_KS_IDLE_WATCHER_H_

#include <QObject>

class KSIdleWatcher : public QObject
{
    Q_OBJECT
public:
    explicit KSIdleWatcher(QObject* parent = nullptr):QObject(parent){};
    virtual ~KSIdleWatcher() = default;

public:
    virtual bool init() = 0;

    virtual bool getActive() = 0;
    virtual bool setActive(bool active) = 0;

    virtual bool getEnabled() = 0;
    virtual bool setEnabled(bool enabled) = 0;

signals:
    /**
     * 空闲状态改变信号
     * \param idle      是否空闲
     * \param handled   标志槽函数是否处理完成
     */
    void idleChanged(bool idle,bool &handled);

     /**
     * 空闲状态通知信号,使用Qt::DirectConnection进行连接，直接调用槽函数进行淡出操作
      * \param isEffect  是否有效，标志是否是处于空闲预告阶段
      * \param handled   标志槽函数是否处理完成
      */
    void idleNoticeChanged(bool isEffect,bool& handled);

};

#endif  //KIRAN_SCREENSAVER_SRC_KS_IDLE_WATCHER_H_
