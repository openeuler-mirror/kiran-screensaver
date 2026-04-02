/**
 * Copyright (c) 2020 ~ 2026 KylinSec Co., Ltd.
 * kiran-screensaver is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 *
 * Author:     liuxinhao <liuxinhao@kylinsec.com.cn>
 */
#include "test-screensaver.h"
#include <qt5-log-i.h>
#include <signal.h>
#include <QDBusInterface>
#include <QApplication>
#include <QLoggingCategory>
#include <QProcess>
#include <QSignalSpy>
#include "common/dbus-session.h"
#include "common/test-utils.h"
#include "common/xephyr-process.h"
#include "kiran_screensaver_proxy.h"
#include "manager.h"

#define DEFAULT_TIMEOUT 2000
#define WAIT_UI_SHOW() QTest::qWait(5000)

namespace Kiran
{
namespace ScreenSaver
{
void TestScreenSaver::initTestCase()
{
    manager = new Manager();
    if (!manager->init())
    {
        qCritical() << "Failed to init manager";
        return;
    }

    kiranScreenSaverProxy = new KiranScreenSaverProxy("com.kylinsec.Kiran.ScreenSaver",
                                                      "/com/kylinsec/Kiran/ScreenSaver",
                                                      QDBusConnection::sessionBus(),
                                                      this);
    if (!kiranScreenSaverProxy->isValid())
    {
        qCritical() << "Failed to create kiran screen saver proxy";
        return;
    }
}

void TestScreenSaver::cleanupTestCase()
{
}

void TestScreenSaver::testInhibit()
{
    inhibitorCookie = kiranScreenSaverProxy->Inhibit("test", "test");
    QVERIFY2(inhibitorCookie != 0, "Foreign cookie is 0");
}

void TestScreenSaver::testGetInhibitors()
{
    // Application="test"; Since="2026-01-09T11:17:24"; Reason="test"
    const QStringList inhibitors = kiranScreenSaverProxy->GetInhibitors();

    QString inhibitorsString = inhibitors.join('\n');
    QVERIFY2(inhibitors.size() == 1, QString("Inhibitors size is not 1: %1").arg(inhibitorsString).toStdString().c_str());

    QString inhibitor = inhibitors.first();

    QString application = inhibitor.split(';').first().split('=').last();
    application = application.remove('\"');
    QVERIFY2(application == "test", QString("Application is not test: %1").arg(application).toStdString().c_str());

    QString reason = inhibitor.split(';').at(2).split('=').last();
    reason = reason.remove('\"');
    QVERIFY2(reason == "test", QString("Reason is not test: %1").arg(reason).toStdString().c_str());
}

void TestScreenSaver::testUninhibit()
{
    auto reply = kiranScreenSaverProxy->UnInhibit(inhibitorCookie);
    QVERIFY2(!reply.isError(), QString("UnInhibit has error: %1").arg(reply.error().message()).toStdString().c_str());

    const QStringList inhibitors = kiranScreenSaverProxy->GetInhibitors();
    QVERIFY2(inhibitors.isEmpty(), QString("Inhibitors is not empty: %1").arg(inhibitors.join('\n')).toStdString().c_str());
}

void TestScreenSaver::testLock()
{
    QSignalSpy spy(kiranScreenSaverProxy, &KiranScreenSaverProxy::ActiveChanged);
    kiranScreenSaverProxy->Lock();

    QVERIFY2(spy.wait(DEFAULT_TIMEOUT), "Failed to wait for ActiveChanged signal");
    QVERIFY2(spy.count() == 1, "ActiveChanged signal count is not 1");
    QVERIFY2(spy.takeFirst().at(0).toBool() == true, "ActiveChanged signal is not true");
    QVERIFY2(kiranScreenSaverProxy->GetActiveTime() != 0, "ActiveTime is 0");

    WAIT_UI_SHOW();
}

void TestScreenSaver::testUnlock()
{
    QSignalSpy spy(kiranScreenSaverProxy, &KiranScreenSaverProxy::ActiveChanged);
    kiranScreenSaverProxy->Unlock();

    QVERIFY2(spy.wait(DEFAULT_TIMEOUT), "Failed to wait for ActiveChanged signal");
    QVERIFY2(spy.count() == 1, "ActiveChanged signal count is not 1");
    QVERIFY2(spy.takeFirst().at(0).toBool() == false, "ActiveChanged signal is not false");

    WAIT_UI_SHOW();
}

void TestScreenSaver::testActivate()
{
    QSignalSpy spy(kiranScreenSaverProxy, &KiranScreenSaverProxy::ActiveChanged);
    kiranScreenSaverProxy->SetActive(true);

    QVERIFY2(spy.wait(DEFAULT_TIMEOUT), "Failed to wait for ActiveChanged signal");
    QVERIFY2(spy.count() == 1, "ActiveChanged signal count is not 1");
    QVERIFY2(spy.takeFirst().at(0).toBool() == true, "ActiveChanged signal is not true");

    WAIT_UI_SHOW();
}

void TestScreenSaver::testDeactivate()
{
    QSignalSpy spy(kiranScreenSaverProxy, &KiranScreenSaverProxy::ActiveChanged);
    kiranScreenSaverProxy->SetActive(false);

    QVERIFY2(spy.wait(DEFAULT_TIMEOUT), "Failed to wait for ActiveChanged signal");
    QVERIFY2(spy.count() == 1, "ActiveChanged signal count is not 1");
    QVERIFY2(spy.takeFirst().at(0).toBool() == false, "ActiveChanged signal is not false");
}
}  // namespace ScreenSaver
}  // namespace Kiran

int main(int argc, char* argv[])
{
    klog_qt5_init(QString(),
                  "kylinsec-session",
                  "kiran-screensaver",
                  QCoreApplication::applicationName());

    QLoggingCategory::setFilterRules("*.debug=false\n"
                                     "*.info=false\n"
                                     "*.warning=false\n"
                                     "*.critical=false");

    bool showHelp = false;
    for (int i = 1; i < argc; ++i)
    {
        QString arg = argv[i];
        if (arg == "--help" || arg == "-h" || arg == "help")
        {
            showHelp = true;
        }
    }

    if (!showHelp)
    {
        TestUtils::initMemoryGSettings();
        if (!XephyrProcess::initXephyr())
        {
            qCritical() << "Failed to init xephyr";
            return EXIT_FAILURE;
        }

        if (!DBusSession::initDBusSession())
        {
            qCritical() << "Failed to create dbus session";
            return EXIT_FAILURE;
        }
    }

    QApplication app(argc, argv);
    Kiran::ScreenSaver::TestScreenSaver testScreenSaver;
    return QTest::qExec(&testScreenSaver, argc, argv);
}
