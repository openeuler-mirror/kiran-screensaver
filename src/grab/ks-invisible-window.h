//
// Created by lxh on 2021/7/7.
//

#ifndef KIRAN_SCREENSAVER_SRC_KS_INVISIBLE_WINDOW_H_
#define KIRAN_SCREENSAVER_SRC_KS_INVISIBLE_WINDOW_H_

#include <QWidget>

class KSInvisibleWindow : public QWidget
{
    Q_OBJECT
public:
    KSInvisibleWindow(QWidget* parent= nullptr);
    ~KSInvisibleWindow();
};

#endif  //KIRAN_SCREENSAVER_SRC_KS_INVISIBLE_WINDOW_H_
