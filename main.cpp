#include "mainwindow.h"
#include <QApplication>
#include "udp_pic5000.h"
#include "camerathread.h"

#include <stdio.h>

#include <QHBoxLayout>
#include <QSplitter>
#include <QLine>
#include <QDebug>
#include <QPixmap>
#include <QImage>
#include <QTime>
#include <math.h>
#include <stdio.h>
#include <assert.h>
#include <time.h>

#if defined (__linux__)
#include <sys/time.h>
#else defined (_WIN32)
#include <Windows.h>
#endif

#include <QMessageBox>
#include <QFileDialog>

#include "settingdialog.h"
#include "tranfertorgb.h"
CameraThread *capThread[NETCAP_CHANNEL_COUNT];
extern volatile int pcapState[NETCAP_CHANNEL_COUNT];
extern char* tifBuff[NETCAP_CHANNEL_COUNT];
extern char* rawBuff[NETCAP_CHANNEL_COUNT];
int index=0;


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    //init
    netcap_init(1);
    capThread[0] = new CameraThread(0);

    //start work
    printf("toggle work %d\n", 0);
    capThread[index]->start();
    Sleep(500);

    while(1)
    {
        //snap picture
        while(netcap_snap(index, 0)!=0){Sleep(500);}

        QString name = QObject::tr("_ch%1-%2").arg(0).arg(QDateTime::currentDateTime().toString("yyyy_MM_dd_hh_mm_ss"))
                .toStdString().c_str();

        while(pcapState[index] != 1){Sleep(100);}
        //pic is done! so show it now
        while(packageCounter[index] < 74211)
        {
            pcapState[index] = 0;
            int count = 0;
            netcap_retrans(index, &count);
            if(count != 0)
            {
                Sleep(500);
            }

            while(pcapState[0] != 1){Sleep(100);};
        }

        pcapState[index] = 0;

        //saveSceneToTiff
        unsigned short line[8304];
        for(int j=0; j<6220; ++j)
        {
            unsigned short *p = (unsigned short*)(rawBuff[0] + 8304*2*j);
            for(int i=0; i<8304/2; ++i)
            {
                line[i] = (*(p+i*2+1) & 16383);
                line[i] *=4;
                line[8303-i] = (*(p+i*2) & 16383);
                line[8303-i] *=4;
            }
            memcpy((char*)p, (const char*)line, 8304*2);
        }
        FILE* fp = fopen(QObject::tr("tiff%1.tif").arg(name).toStdString().c_str(), "wb");
        fwrite(tifBuff[index], 1, 103301760+8+138, fp);
        fclose(fp);

        Sleep(1000);
    }

    //stop work
    capThread[0]->CameraThread::stopCamera();


    return a.exec();

}
