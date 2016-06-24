#include "camerathread.h"
#include "udp_pic5000.h"

#include <QSettings>
#include <stdio.h>
#include <QDebug>

void CameraThread::run()
{

    printf("start channel %d\n", channel);
    QSettings setting("pic5000.ini", QSettings::IniFormat);
    QString devname = setting.value(tr("/net/channel%1")
                                    .arg(channel), "").toString();
    qDebug() << devname;

    if(!devname.isEmpty())
        netcap_thread(channel, devname.toStdString().c_str());

}

CameraThread::CameraThread(int index)
    :channel(index)
{

}

void CameraThread::stopCamera()
{
    printf("stopping channel %d\n", channel);
    netcap_stop(channel);

}
