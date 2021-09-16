//
// Created by lxh on 2021/7/7.
//

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
