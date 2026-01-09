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
#pragma once
#include <QObject>
#include <QProcess>
#include <QTest>


class KiranScreenSaverProxy;
namespace Kiran
{
namespace ScreenSaver
{
class Manager;
class TestScreenSaver : public QObject
{
    Q_OBJECT
private slots:
    void initTestCase();
    void cleanupTestCase();

    void testInhibit();
    void testGetInhibitors();
    void testUninhibit();

    void testLock();
    void testUnlock();
    
    void testActivate();
    void testDeactivate();
public:
    Manager* manager;
    KiranScreenSaverProxy* kiranScreenSaverProxy;
    uint inhibitorCookie = 0;
};

}  // namespace ScreenSaver
}  // namespace Kiran