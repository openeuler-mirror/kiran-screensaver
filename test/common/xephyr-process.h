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
#include <QProcess>

class XephyrProcess : private QProcess
{
    Q_OBJECT
public:
    XephyrProcess(QObject* parent = nullptr) : QProcess(parent){};
    virtual ~XephyrProcess();

    // 初始化一个Xephyr内嵌Xserver进程，用于对单元测试提供独立的X环境
    // 返回值：是否成功初始化
    // 注意：
    // 1. 这个函数会设置DISPLAY环境变量，所以需要在使用前调用
    // 2. Xephyr进程拉起后，会在测试程序退出后销毁
    static bool initXephyr();

private:
    virtual void setupChildProcess();

private:
    static bool isXephyrReady;
    static XephyrProcess* xephyrProcess;
};
