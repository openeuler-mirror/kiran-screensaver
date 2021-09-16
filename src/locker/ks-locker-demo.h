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
public:
    explicit KSLockerDemo(bool enableAnimation, QWidget* parent = nullptr);
    ~KSLockerDemo() override;

    bool eventFilter(QObject* watched, QEvent* event) override;

    // 是否可见
    bool fadeVisible();
    // 淡入
    void fadeIn();
    // 淡出
    void fadeOut();

private:
    void init();
    void adjustGeometry(const QSize& size);

protected:
    void resizeEvent(QResizeEvent* event) override;
    void changeEvent(QEvent* event) override;

signals:
    void authenticationPassed();

private:
    Ui::KSLockerDemo* ui;
    QWidget* m_parentWidget = nullptr;
    bool m_fadeVisible = false;
    bool m_enableAnimation = false;
    QPropertyAnimation* m_animation = nullptr;
    QGraphicsOpacityEffect* m_opacityEffect = nullptr;
};

#endif  //KIRAN_SCREENSAVER_SRC_LOCKER_LOCKERDEMO_H_
