//
// Created by lxh on 2021/7/7.
//

#ifndef KIRAN_SCREENSAVER_TEST_FADE_FADE_TEST_H_
#define KIRAN_SCREENSAVER_TEST_FADE_FADE_TEST_H_

#include <QObject>

class KSFade;
class FadeTest : public QObject
{
    Q_OBJECT
public:
    FadeTest(QObject* parent= nullptr);
    ~FadeTest();

    bool start();

private:
    KSFade* m_fade = nullptr;
};

#endif  //KIRAN_SCREENSAVER_TEST_FADE_FADE_TEST_H_
