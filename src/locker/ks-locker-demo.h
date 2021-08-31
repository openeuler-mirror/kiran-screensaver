//
// Created by lxh on 2021/8/30.
//

#ifndef KIRAN_SCREENSAVER_SRC_LOCKER_LOCKERDEMO_H_
#define KIRAN_SCREENSAVER_SRC_LOCKER_LOCKERDEMO_H_

#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui
{
class KSLockerDemo;
}
QT_END_NAMESPACE

class QPropertyAnimation;
class QGraphicsOpacityEffect;
class KSLockerDemo : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(bool active READ active WRITE setActive)
public:
    explicit KSLockerDemo(bool enableAnimation, QWidget* parent = nullptr);
    ~KSLockerDemo() override;

    bool eventFilter(QObject *watched, QEvent *event) override;

    bool active() const;
    void setActive(bool active);

private:
    void init();

protected:
    void resizeEvent(QResizeEvent* event) override;
    void changeEvent(QEvent *event) override;

private:
    Ui::KSLockerDemo *ui;
    QWidget* m_parentWidget = nullptr;
    bool m_active = false;
    bool m_enableAnimation = false;
    QPropertyAnimation* m_animation = nullptr;
    QGraphicsOpacityEffect* m_opacityEffect;
    void adjustGeometry(const QSize& size);
};

#endif  //KIRAN_SCREENSAVER_SRC_LOCKER_LOCKERDEMO_H_
