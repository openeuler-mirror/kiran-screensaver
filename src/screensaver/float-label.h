//
// Created by lxh on 2021/9/28.
//

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
