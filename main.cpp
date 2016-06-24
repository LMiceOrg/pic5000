#include "mainwindow.h"
#include <QApplication>
#include "udp_pic5000.h"
#include "camerathread.h"

#include <stdio.h>
#include <QTranslator>

CameraThread *capThread[NETCAP_CHANNEL_COUNT];
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QTranslator trans;
    trans.load(":/trans/zh_cn");
    a.installTranslator(&trans);
    netcap_init(1);
    capThread[0] = new CameraThread(0);

    MainWindow w;
    w.show();
    return a.exec();
}
