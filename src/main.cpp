#include <kiran-application.h>

#include "ks-manager.h"
#include "qt5-log-i.h"

int main(int argc, char *argv[])
{
    klog_qt5_init("",
                  "kylinsec-session",
                  "kiran-screensaver",
                  "kiran-screensaver");

    KiranApplication app(argc, argv);

    KSManager manager;
    if( !manager.init() )
    {
        KLOG_ERROR() << "kiran-screensaver init failed! exit.";
        return EXIT_FAILURE;
    }

    return QApplication::exec();
}
