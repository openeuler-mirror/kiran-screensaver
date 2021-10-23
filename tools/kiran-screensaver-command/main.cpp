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

#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QCoreApplication>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDebug>
#include <QPair>
#include <QDBusConnectionInterface>
#include <QTranslator>

#define SCREENSAVER_SERVICE "com.kylinsec.Kiran.ScreenSaver"
#define SCREENSAVER_PATH "/com/kylinsec/Kiran/ScreenSaver"
#define SCREENSAVER_INTERFACE "com.kylinsec.Kiran.ScreenSaver"

static QDBusMessage callScreenSaverDBusMethodVoid(const QString& method)
{
    QDBusMessage methodCall = QDBusMessage::createMethodCall(SCREENSAVER_SERVICE,SCREENSAVER_PATH,SCREENSAVER_INTERFACE,method);
    return QDBusConnection::sessionBus().call(methodCall);
}

static QDBusMessage callScreenSaverDBusMethodBool(const QString& method,bool b)
{
    QDBusMessage methodCall = QDBusMessage::createMethodCall(SCREENSAVER_SERVICE,SCREENSAVER_PATH,SCREENSAVER_INTERFACE,method);
    methodCall.setArguments(QList<QVariant>() << b);
    return QDBusConnection::sessionBus().call(methodCall);
}

static QDBusMessage callScreenSaverInhibit(const QString& app,const QString& reason)
{
    QDBusMessage methodCall = QDBusMessage::createMethodCall(SCREENSAVER_SERVICE,SCREENSAVER_PATH,SCREENSAVER_INTERFACE,"Inhibit");
    methodCall.setArguments(QList<QVariant>() << app << reason );
    return QDBusConnection::sessionBus().call(methodCall);
}

static bool screenSaverIsRunning()
{
    auto reply = QDBusConnection::sessionBus().interface()->isServiceRegistered(SCREENSAVER_SERVICE);
    bool isRunning = reply.value();
    return isRunning;
}

int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);

    auto translator = new QTranslator;
    if( translator->load(QLocale(),
                     "kiran-screensaver-command",
                     ".",
                     "/usr/share/kiran-screensaver/translations/",
                     ".qm") )
    {
        app.installTranslator(translator);
    }
    else
    {
        qWarning() << "can't load translator";
    }

    QCommandLineOption query({"query","q"},QCoreApplication::tr("Query the state of the screensaver"));
    QCommandLineOption time({"time", "t"},QCoreApplication::tr("Query the length of time the screensaver has been active"));
    QCommandLineOption lock({"lock", "l"},QCoreApplication::tr("Tells the running screensaver process to lock the screen immediately"));
    QCommandLineOption unlock({"unlock", "u"},QCoreApplication::tr("Tells the running screensaver process to unlock the screen immediately"));
    QCommandLineOption activate({"activate","a"},QCoreApplication::tr("Turn the screensaver on"));
    QCommandLineOption deactivate({"deactivate","d"},QCoreApplication::tr("If the screensaver is active then deactivate it"));
    QCommandLineOption inhibit({"inhibit","i",},QCoreApplication::tr("Inhibit the screensaver from activating.  Command blocks while inhibit is active."));
    QCommandLineOption applicationName({"application-name","n"},QCoreApplication::tr("The calling application that is inhibiting the screensaver"),"application");
    QCommandLineOption reasion({"reason","r"},QCoreApplication::tr("The reason for inhibiting the screensaver"));

    QCommandLineParser commandParser;
    commandParser.addOptions({query,time,lock,unlock,activate,deactivate,inhibit,applicationName,reasion});
    commandParser.addHelpOption();
    commandParser.process(app);

    QString inhibitorReason="Unknow",inhibitorAppName="Unknow";
    QString tips;

    if(!screenSaverIsRunning())
    {
        tips = QCoreApplication::tr("Kiran ScreenSaver is not running!");
        qWarning("%s", tips.toStdString().c_str());
        return  EXIT_FAILURE;
    }

    if(commandParser.isSet(query))
    {
        auto activeReply = callScreenSaverDBusMethodVoid("GetActive");
        if(activeReply.type() == QDBusMessage::ErrorMessage)
        {
            tips = QCoreApplication::tr("call 'GetActive' failed,%1.").arg(activeReply.errorMessage());
            qCritical("%s",tips.toStdString().c_str());
            goto err_done;
        }
        bool activeState = activeReply.arguments().value(0).toBool();

        tips = QCoreApplication::tr("The screensaver is %1.").arg(activeState?QCoreApplication::tr("active"):QCoreApplication::tr("inactive"));
        qInfo("%s",tips.toStdString().c_str());

        auto inhibitorsReply = callScreenSaverDBusMethodVoid("GetInhibitors");
        if(inhibitorsReply.type() == QDBusMessage::ErrorMessage)
        {
            tips = QCoreApplication::tr("call 'GetInhibitors' failed,%1.",inhibitorsReply.errorMessage().toStdString().c_str());
            qCritical("%s",tips.toStdString().c_str());
        }
        QDBusReply<QStringList> inhibitorsDBusReply(inhibitorsReply);
        if(inhibitorsDBusReply.value().isEmpty())
        {
            qInfo("%s",QCoreApplication::tr("The screensaver is not inhibited.").toStdString().c_str());
        }
        else
        {
            qInfo("%s",QCoreApplication::tr("The screensaver is being inhibited by:").toStdString().c_str());
            for(const QString& inhibitor:inhibitorsDBusReply.value())
            {
                qInfo("\t%s",inhibitor.toStdString().c_str());
            }
        }
    }

    if(commandParser.isSet(time))
    {
        auto activeReply = callScreenSaverDBusMethodVoid("GetActive");
        if(activeReply.type() == QDBusMessage::ErrorMessage)
        {
            tips = QCoreApplication::tr("call 'GetActive' failed,%1").arg(activeReply.errorMessage());
            qCritical("%s",tips.toStdString().c_str());
            goto err_done;
        }

        bool activeState = activeReply.arguments().value(0).toBool();
        if(!activeState)
        {
            tips = QCoreApplication::tr("The screensaver is not currently active.");
            qInfo("%s",tips.toStdString().c_str());
        }
        else
        {
            auto activeTimeReply = callScreenSaverDBusMethodVoid("GetActiveTime");
            if(activeTimeReply.type() == QDBusMessage::ErrorMessage)
            {
                tips = QCoreApplication::tr("call 'GetActiveTime' failed,%1").arg(activeReply.errorMessage());
                qCritical("%s",tips.toStdString().c_str());
                goto err_done;
            }
            uint activeTime = activeTimeReply.arguments().at(0).toUInt();
            qInfo("The screensaver has been active for %d seconds.",activeTime);
        }
    }

    if(commandParser.isSet(lock))
    {
        QDBusMessage reply = callScreenSaverDBusMethodVoid("Lock");
    }

    if(commandParser.isSet(unlock))
    {
        QDBusMessage reply = callScreenSaverDBusMethodVoid("Unlock");
    }

    if(commandParser.isSet(activate))
    {
        QDBusMessage reply = callScreenSaverDBusMethodBool("SetActive",true);
    }

    if(commandParser.isSet(deactivate))
    {
        QDBusMessage reply = callScreenSaverDBusMethodBool("SetActive",false);
    }

    if(commandParser.isSet(applicationName))
    {
        inhibitorAppName = commandParser.value(applicationName);
    }

    if(commandParser.isSet(reasion))
    {
        inhibitorReason = commandParser.value(reasion);
    }

    if(commandParser.isSet(inhibit))
    {
        QDBusMessage reply = callScreenSaverInhibit(inhibitorAppName,inhibitorReason);
        qInfo() << reply;
        return app.exec();
    }

    return EXIT_SUCCESS;
err_done:
    return EXIT_FAILURE;
}