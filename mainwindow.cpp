#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "view.h"

#include <QHBoxLayout>
#include <QSplitter>
#include <QLine>
#include <QDebug>
#include <QPixmap>
#include <QImage>
#include <QTime>
#include <math.h>
#include <stdio.h>
//#include <stdint.h>
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
//TIFF lib
//#include "tiffio.h"
#include "udp_pic5000.h"

#if defined(__linux__)
struct timeval totalStartTime[NETCAP_CHANNEL_COUNT];
struct timeval totalEndTime[NETCAP_CHANNEL_COUNT];
#elif defined(_WIN32)
#include <Windows.h>
DWORD totalStartTime[NETCAP_CHANNEL_COUNT];
DWORD totalEndTime[NETCAP_CHANNEL_COUNT];

#else
#error("No implementation!")
#endif

inline ushort ntohs_i(ushort x)
{
    x = x&16383;
    //    char*p = (char*)&x;
    //    char c = *(p+1);
    //    *(p+1) = *p;
    //    *p = c;
    return x;
}
inline ushort ntohs_i2(ushort x)
{
    x = x&16383;
    //    char*p = (char*)&x;
    //    char c = *(p+1);
    //    *(p+1) = *p;
    //    *p = c;
    return x;
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    /// setup scene
    for(int i=0; i<NETCAP_CHANNEL_COUNT; ++i)
    {
        scene[i] = new QGraphicsScene(this);
//        resetScene(i);
    }

    ui->widget->setVisible(false);
    h1Splitter = new QSplitter(ui->centralWidget);
    h2Splitter = new QSplitter(ui->centralWidget);
    vSplitter = new QSplitter(ui->centralWidget);
    vSplitter->setOrientation(Qt::Vertical);
    vSplitter->addWidget(h1Splitter);
    vSplitter->addWidget(h2Splitter);

    /// setup views
    View *view;

    view = new View(tr("Top left view"), 0);
    view->view()->setScene(scene[0]);
    h1Splitter->addWidget(view);
    connect(view, SIGNAL(Maximized(int)), this, SLOT(toggleViewMaximize(int)) );
    connect(view, SIGNAL(Normalized(int)), this, SLOT(toggleViewNormalize(int)) );
    connect(view, SIGNAL(ResetPixmap(int)), this, SLOT(resetScene(int)));
    connect(view, SIGNAL(SaveRGBImage(int)), this, SLOT(changeToRGB(int)));
    connect(view, SIGNAL(StartSnapPicture(int)), this, SLOT(startSnapPicture(int)));
    connect(this, SIGNAL(currentFileName(QString)),
            view, SLOT(changeCurrentFileName(QString)) );

    view = new View(tr("Top right view"), 1);
    view->view()->setScene(scene[1]);
    h1Splitter->addWidget(view);
    connect(view, SIGNAL(Maximized(int)), this, SLOT(toggleViewMaximize(int)) );
    connect(view, SIGNAL(Normalized(int)), this, SLOT(toggleViewNormalize(int)) );
    connect(view, SIGNAL(ResetPixmap(int)), this, SLOT(resetScene(int)));
    connect(view, SIGNAL(SaveRGBImage(int)), this, SLOT(changeToRGB(int)));
    connect(view, SIGNAL(StartSnapPicture(int)), this, SLOT(startSnapPicture(int)));
    connect(this, SIGNAL(currentFileName(QString)),
            view, SLOT(changeCurrentFileName(QString)) );

    view = new View(tr("Bottom left view"), 2);
    view->view()->setScene(scene[2]);
    h2Splitter->addWidget(view);
    connect(view, SIGNAL(Maximized(int)), this, SLOT(toggleViewMaximize(int)) );
    connect(view, SIGNAL(Normalized(int)), this, SLOT(toggleViewNormalize(int)) );
    connect(view, SIGNAL(ResetPixmap(int)), this, SLOT(resetScene(int)));
    connect(view, SIGNAL(StartSnapPicture(int)), this, SLOT(startSnapPicture(int)));

    view = new View(tr("Bottom right view"), 3);
    view->view()->setScene(scene[3]);
    h2Splitter->addWidget(view);
    connect(view, SIGNAL(Maximized(int)), this, SLOT(toggleViewMaximize(int)) );
    connect(view, SIGNAL(Normalized(int)), this, SLOT(toggleViewNormalize(int)) );
    connect(view, SIGNAL(ResetPixmap(int)), this, SLOT(resetScene(int)));
    connect(view, SIGNAL(StartSnapPicture(int)), this, SLOT(startSnapPicture(int)));

    QHBoxLayout *layout = new QHBoxLayout;
    layout->addWidget(vSplitter);
    ui->centralWidget->setLayout(layout);

    setWindowTitle(tr("5000 X 4 camera"));
    setWindowState(Qt::WindowMaximized);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::resetScene(int index)
{



    QString name = tr("_ch%1-%2").arg(index).arg(QDateTime::currentDateTime().toString("yyyy_MM_dd_hh_mm_ss"))
            .toStdString().c_str();
    emit currentFileName(name);
//    FILE* raw = fopen(tr("raw%1.bin").arg(name).toStdString().c_str(), "wb");
//    fwrite(rawBuff[index], 1, 103301760, raw);
//    fclose(raw);

    unsigned short line[8304];
    for(int j=0; j<6220; ++j)
    {
        unsigned short *p = (unsigned short*)(rawBuff[index] + 8304*2*j);
        for(int i=0; i<8304/2; ++i)
        {
            line[i] = (*(p+i*2+1) & 16383);
            line[i] *=4;
            line[8303-i] = (*(p+i*2) & 16383);
            line[8303-i] *=4;
        }
        memcpy((char*)p, (const char*)line, 8304*2);
    }

    scene[index]->clear();

    QPixmap pi;//("output_full.tif", "TIF");
    bool bload = pi.loadFromData( (unsigned char*)(tifBuff[0]), 8304*6220*2+8+138, "TIF" );
    if(!bload)
        printf("index %d load failed\n", index);

    scene[index]->addPixmap(pi);
#if defined(__linux__)
    struct timezone tz;
    struct timeval t1,t2;
    gettimeofday(&t1, &tz);
    saveSceneToTiff(index, name);
    gettimeofday(&t2, &tz);

    gettimeofday(& totalEndTime[index], &tz);
    setWindowTitle(tr("time[%1]:packages=%2 trans=%3  store=%4 total=%5")
                   .arg(QDateTime::currentDateTime().toString("hhmmss"))
                   .arg(packageCounter[index])
                   .arg( difftimeval(packageEndTime[index], packageStartTime[index]) )
                   .arg( difftimeval(t2,t1))
                   .arg( difftimeval(totalEndTime[index], totalStartTime[index]))
                   );
    printf("packages=%d trans=%lld store=%lld total=%lld\n",
           packageCounter[index],
           difftimeval(packageEndTime[index], packageStartTime[index]),
           difftimeval(t2,t1),
           difftimeval(totalEndTime[index], totalStartTime[index]));
#else defined(_WIN32)
    DWORD t1, t2;
    t1 = GetTickCount();
    saveSceneToTiff(index, name);
    t2 = GetTickCount();

    totalEndTime[index] = GetTickCount();
    setWindowTitle(tr("time[%1]:packages=%2 trans=%3  store=%4 total=%5")
                   .arg(QDateTime::currentDateTime().toString("hhmmss"))
                   .arg(packageCounter[index])
                   .arg( packageEndTime[index]- packageStartTime[index])
                   .arg( t2-t1)
                   .arg( totalEndTime[index]- totalStartTime[index])
                   );

#endif
    if(packageCounter[index] < 74211)
    {
        int count = 0;
        netcap_retrans(index, &count);
        if(count != 0)
        {
#if defined (__linux__)
            timespec ts1,ts2;
            ts1.tv_sec=0;
            ts1.tv_nsec = 500*1e6;
            nanosleep(&ts1, &ts2);
#else  defined(_WIN32)
            Sleep(500);
#endif
        }
        printf("retrans count is %d\n", count);
#if defined (__linux__)
        printf("packages=%d trans=%lld store=%lld total=%lld\n",
               packageCounter[index],
               difftimeval(packageEndTime[index], packageStartTime[index]),
               difftimeval(t2,t1),
               difftimeval(totalEndTime[index], totalStartTime[index]));
#else  defined(_WIN32)
        printf("packages=%d trans=%lld store=%lld total=%lld\n",
               packageCounter[index],
               packageEndTime[index]-packageStartTime[index],
               t2-t1,
               totalEndTime[index]-totalStartTime[index]);
#endif
    }
    packageCounter[index] = 0;
    memset(rawBuff[index], 0, 103301760);
}

void MainWindow::saveSceneToTiff(int index,const QString name)
{
    (void)index;
    FILE* fp = fopen(tr("tiff%1.tif").arg(name).toStdString().c_str(), "wb");
    fwrite(tifBuff[index], 1, 103301760+8+138, fp);
    fclose(fp);
}

inline unsigned int get_rp(int i, int j)
{
    unsigned char* row_pointers = (unsigned char*)rawBuff[0];
    unsigned int low, high;
    low = (unsigned char) *(row_pointers+(i)*8304*2 +(j)*2);
    high = (unsigned char)*(row_pointers+(i)*8304*2 +(j)*2 +1 );
    return low + high  * 256;
}

void MainWindow::changeToRGB(int index)
{
//    (void)index;
////    {
////        FILE *fp = fopen("tiff_ch0-2014_07_10_18_13_20.tif", "rb");
////        fread(tifBuff[0], 1, 103301760 +8 + 138, fp);
////        fclose(fp);
////    }
//    char *colorData = NULL;
//    int i,j,k;
//    unsigned int red, green, blue, blue_u, blue_d, blue_l, blue_r, red_u, red_d, red_l, red_r,green_lr, green_ud;
//    int sel;
//    bool bg=false;
//    bool bg_b=true;
//    bool rg_g=true;
//   // unsigned char* row_pointers = (unsigned char*)rawBuff[0];

//    colorData = (char*)malloc(6132*8176*3);
//   // char* row_pointers_c;

//    for(i=33; i<6165; i++)
//    {
//        k=0;
//        for(j=53; j<8229; j++)
//        {
//            blue=0;
//            green=0;
//            red=0;
//            if(bg)//blue and green
//            {
//                if(bg_b)//pos blue
//                {
//                    red=    get_rp(i-1, j-1)+
//                            get_rp(i-1, j+1)+
//                            get_rp(i+1, j-1)+
//                            get_rp(i+1, j+1);
//                    red/=4;

//                    blue_u= get_rp(i-2, j);
//                            //row_pointers[i-2][j*2]*256+row_pointers[i-2][j*2+1];//up
//                    blue_d= get_rp(i+2, j);
//                            //row_pointers[i+2][j*2]*256+row_pointers[i+2][j*2+1];//down
//                    blue_l= get_rp(i, j-2);
//                             //row_pointers[i][(j-2)*2]*256+row_pointers[i][(j-2)*2+1];//left
//                    blue_r=get_rp(i, j+2);
//                            //row_pointers[i][(j+2)*2]*256+row_pointers[i][(j+2)*2+1];//right

//                    sel=(int)(fabs((double)(blue_u - blue_d)) - fabs((double)(blue_r - blue_l)));

//                    green_ud=get_rp(i-1, j )+get_rp(i+1, j);
//                            //row_pointers[i-1][j*2]*256+row_pointers[i-1][j*2+1]//up
//                            //+row_pointers[i+1][j*2]*256+row_pointers[i+1][j*2+1];//down
//                    green_lr=get_rp(i, j-1)+get_rp(i,j+1);
//                    //row_pointers[i][(j-1)*2]*256+row_pointers[i][(j-1)*2+1]//left
//                      //      +row_pointers[i][(j+1)*2]*256+row_pointers[i][(j+1)*2+1];//right
//                    if(sel>0)
//                        green=green_lr/2;
//                    else if(sel<0)
//                        green=green_ud/2;
//                    else
//                        green=(green_ud+green_lr)/4;

//                    blue=get_rp(i, j);
//                        //row_pointers[i][j*2]*256+row_pointers[i][j*2+1];
//                    bg_b=!bg_b;
//                }
//                else//pos green
//                {
//                    red=get_rp(i-1, j) + get_rp(i+1, j);
//                            //row_pointers[i-1][j*2]*256+row_pointers[i-1][j*2+1]//up
//                       //+row_pointers[i+1][j*2]*256+row_pointers[i+1][j*2+1];//down
//                    red/=2;

//                    green=get_rp(i, j);
//                    //row_pointers[i][j*2]*256+row_pointers[i][j*2+1];

//                    blue=get_rp(i,j-1) + get_rp(i, j+1);
//                            //row_pointers[i][(j-1)*2]*256+row_pointers[i][(j-1)*2+1]//left
//                        //+row_pointers[i][(j+1)*2]*256+row_pointers[i][(j+1)*2+1];//right
//                    blue/=2;
//                    bg_b=!bg_b;
//                }
//            }
//            else
//            {
//                if(rg_g)//pos green
//                {
//                    blue=get_rp(i-1, j) + get_rp(i+1, j);
//                    //row_pointers[i-1][j*2]*256+row_pointers[i-1][j*2+1]//up
//                      //  +row_pointers[i+1][j*2]*256+row_pointers[i+1][j*2+1];//down
//                    blue/=2;

//                    green=get_rp(i ,j );
//                            //row_pointers[i][j*2]*256+row_pointers[i][j*2+1];

//                    red=get_rp(i, j-1) + get_rp(i, j+1);
//                    //row_pointers[i][(j-1)*2]*256+row_pointers[i][(j-1)*2+1]//left
//                      // +row_pointers[i][(j+1)*2]*256+row_pointers[i][(j+1)*2+1];//right
//                    red/=2;
//                    rg_g=!rg_g;
//                }
//                else//pos red
//                {
//                    red=get_rp(i,j);//row_pointers[i][j*2]*256+row_pointers[i][j*2+1];

//                    blue=   get_rp(i-1, j-1) +
//                            get_rp(i-1, j+1) +
//                            get_rp(i+1, j-1) +
//                            get_rp(i+1, j+1);/*row_pointers[i-1][(j-1)*2]*256+row_pointers[i-1][(j-1)*2+1]//up_left
//                        +row_pointers[i-1][(j+1)*2]*256+row_pointers[i-1][(j+1)*2+1]//up_right
//                        +row_pointers[i+1][(j-1)*2]*256+row_pointers[i+1][(j-1)*2+1]//down_left
//                        +row_pointers[i+1][(j+1)*2]*256+row_pointers[i+1][(j+1)*2+1];//down_right*/
//                    blue/=4;

//                    red_u=get_rp(i-2, j);//row_pointers[i-2][j*2]*256+row_pointers[i-2][j*2+1];//up
//                    red_d=get_rp(i+2, j);//row_pointers[i+2][j*2]*256+row_pointers[i+2][j*2+1];//down
//                    red_l=get_rp(i, j-2);//row_pointers[i][(j-2)*2]*256+row_pointers[i][(j-2)*2+1];//left
//                    red_r=get_rp(i, j+2);//row_pointers[i][(j+2)*2]*256+row_pointers[i][(j+2)*2+1];//right
//                    sel=(int)(fabs((double)(red_u - red_d)) - fabs((double)(red_r - red_l)));

//                    green_ud=get_rp(i-1, j) +get_rp(i+1, j);
//                    //row_pointers[i-1][j*2]*256+row_pointers[i-1][j*2+1]//up
//                      //   +row_pointers[i+1][j*2]*256+row_pointers[i+1][j*2+1];//down
//                    green_lr=get_rp(i, j-1)+get_rp(i, j+1);//row_pointers[i][(j-1)*2]*256+row_pointers[i][(j-1)*2+1]//left
//                         //+row_pointers[i][(j+1)*2]*256+row_pointers[i][(j+1)*2+1];//right
//                    if(sel>0)
//                        green=green_lr/2;
//                    else if(sel<0)
//                        green=green_ud/2;
//                    else
//                        green=(green_ud+green_lr)/4;
//                    rg_g=!rg_g;
//                }
//            }
//            //row_pointers_c = colorData + 8176*2*(i-33);
//            //*(unsigned short*)( colorData + 8176*2*(i-33)+k) = ((red>>11)<<10) + ((green>>11)<<5) + (blue>>11);
//            assert(red < 65535 );
//            assert(green < 65535 );
//            assert(blue < 65535 );
//            *( colorData + 8176*3*(i-33)+k) = (red >>8);
//            *( colorData + 8176*3*(i-33)+k+1) = (green >>8 );
//            *( colorData + 8176*3*(i-33)+k+2) = (blue >> 8);

////            row_pointers_c[i-33][k] = (char)(red>>8); // red
////            row_pointers_c[i-33][k+1] = (char)red; // red

////            row_pointers_c[i-33][k+2] = (char)(green>>8); // green
////            row_pointers_c[i-33][k+3] = (char)green; // green

////            row_pointers_c[i-33][k+4] = (char)(blue>>8); // blue
////            row_pointers_c[i-33][k+5] = (char)blue; // blue
//            k+=3;
//        }
//        bg=!bg;
//    }
//    //printf("index: i=%d, j=%d\n",i, j);
//    QImage img((unsigned char*)colorData, 8176, 6132, QImage::Format_RGB888);
//    //QImage img((unsigned char*)colorData, 8176, 6132, QImage::Format_RGB555);
//    img.save(tr("cc.bmp"));
//    free(colorData);
    pc_main(index);
    scene[index]->clear();
    QPixmap pic;
    pic.load("a888.bmp");
    scene[index]->addPixmap(pic);
}

void MainWindow::startSnapPicture(int index)
{
    (void)index;
    setWindowTitle(tr("Begin snapping"));
}

void MainWindow::toggleViewMaximize(int i)
{
    qDebug()<<"max button slot"<<i;
    spliterStates[0] = h1Splitter->saveState();
    spliterStates[1] = h2Splitter->saveState();
    spliterStates[2] = vSplitter->saveState();
    QList<int> szh1 = h1Splitter->sizes();
    QList<int> szh2 = h2Splitter->sizes();
    QList<int> szv = vSplitter->sizes();


    switch(i)
    {
    case 0:
        szh1[0] += szh1[1];
        szh1[1] = 0;
        h1Splitter->setSizes(szh1);
        szv[0] += szv[1];
        szv[1] = 0;
        vSplitter->setSizes(szv);

        break;
    case 1:
        szh1[1] += szh1[0];
        szh1[0] = 0;
        h1Splitter->setSizes(szh1);
        szv[0] += szv[1];
        szv[1] = 0;
        vSplitter->setSizes(szv);
        break;
    case 2:

        szh2[0] += szh2[1];
        szh2[1] = 0;
        h2Splitter->setSizes(szh2);
        szv[1] += szv[0];
        szv[0] = 0;
        vSplitter->setSizes(szv);
        break;
    case 3:

        szh2[1] += szh2[0];
        szh2[0] = 0;
        h2Splitter->setSizes(szh2);
        szv[1] += szv[0];
        szv[0] = 0;
        vSplitter->setSizes(szv);
        break;
    }


}

void MainWindow::toggleViewNormalize(int i)
{
    (void)i;

    h1Splitter->restoreState(spliterStates[0]);
    h2Splitter->restoreState(spliterStates[1]);
    vSplitter->restoreState(spliterStates[2]);

}

void MainWindow::on_pushButton_clicked()
{
    ui->lineEdit->setText( tr("%1").arg(packageCounter[0]) );
}

void MainWindow::on_pushButton_2_clicked()
{
    ushort *p = (ushort*)rawBuff[0];
    ushort line[8304];
    QTime c1 = QTime::currentTime();
    int c;

    ;
    //8304x6220 交错替换
    for(int j=0; j<6220; ++j)
    {
        for(int i=0; i<8304/2;++i)
        {
            line[i]      = ntohs_i(*(p+i*2+1));
            c=line[i]/512;;
            //c=pow(2, (int)(log2(line[i])/2.8+0.5) );
            c = c & 31;
            line[i] = (c<<5)+(c<<10)+c;

            line[8303-i]    = ntohs_i2(*(p+i*2) );
            c=line[8303-i]/512;
            //`c=pow(2, (int)(log2(line[8303-i])/2.8+0.5) );//line[8303-i]/512;
            c = c & 31;
            line[8303-i] = (c<<5)+(c<<10)+c;
        }
        memcpy(rawBuff[0]+j*8304*2, line, 8304*2);
        p+= 8304;
    }
    QImage img((unsigned char*)rawBuff[0], 8304, 6220, QImage::Format_RGB555);
    bool bsave = img.save(tr("a.bmp"));
    if(!bsave)
    {
        QMessageBox box;
        box.setText("not saved");
        box.exec();
    }
    else
    {
        QMessageBox box;
        box.setText(tr("done %1")
                    .arg( QTime::currentTime().msecsSinceStartOfDay() - c1.msecsSinceStartOfDay() ));
        box.exec();
    }
    packageCounter[0] = 0;
}

void MainWindow::on_pushButton_3_clicked()
{
    FILE * f = fopen("a.bin", "wb");
    fwrite(rawBuff[0], 1, 8304*6220*2, f);
    fclose(f);
}

void MainWindow::on_pushButton_4_clicked()
{
    FILE* f = fopen("a.bin", "rb");
    fread(rawBuff[0], 1, 8304*6220*2, f);
    fclose(f);

}

//struct tiff_header_t
//{
//    //49 49 2a 00
//    uint32_t tag;
//    uint32_t length; //pixel bytes count + header bytes count
//};

//struct tiff_tail_t
//{
//    //0b 00
//    uint8_t tag[2];
//    //00 01 03 00
//    //01 00 00 00
//    uint32_t tag1[2];
//    uint32_t pic_width;
//    //01 01 03 00
//    //01 00 00 00
//    uint32_t tag2[2];
//    uint32_t pic_height;
//    //02 01 03 00
//    //01 00 00 00
//    //10 00 00 00
//    //03 01 03 00
//    //01 00 00 00
//    //01 00 00 00
//    //06 01 03 00
//    //01 00 00 00
//    //01 00 00 00
//    //0a 01 03 00
//    //01 00 00 00
//    //01 00 00 00
//    //11 01 04 00
//    //01 00 00 00
//    //08 00 00 00
//    //15 01 03 00
//    //01 00 00 00
//    //01 00 00 00
//    //16 01 03 00
//    //01 00 00 00
//    uint32_t tag3[20];
//    uint32_t pic_height2;//same as pic_height
//    //17 01 04 00
//    //01 00 00 00
//    uint32_t tag4[2];
//    uint32_t pic_length;//pixel bytes count
//    //1c 01 03 00
//    //01 00 00 00
//    //01 00 00 00
//    //00 00 00 00
//    uint32_t tag5[4];
//};

void MainWindow::on_pushButton_5_clicked()
{
    QTime c1 = QTime::currentTime();
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

    //    TIFF *image;

    //    // Open the TIFF file
    //    if((image = TIFFOpen("output_full.bin2", "w")) == NULL){
    //        printf("Could not open output.tif for writing\n");
    //        exit(42);
    //    }
    //    // We need to set some values for basic tags before we can add any data
    //    TIFFSetField(image, TIFFTAG_IMAGEWIDTH, 8304 );
    //    TIFFSetField(image, TIFFTAG_IMAGELENGTH, 6220);
    //    TIFFSetField(image, TIFFTAG_BITSPERSAMPLE, 16);
    //    TIFFSetField(image, TIFFTAG_SAMPLESPERPIXEL, 1);
    //    TIFFSetField(image, TIFFTAG_ROWSPERSTRIP, 6220);
    //    TIFFSetField(image, TIFFTAG_COMPRESSION, COMPRESSION_NONE);
    //    //TIFFSetField(image, TIFFTAG_COMPRESSION, COMPRESSION_DEFLATE);
    //    //TIFFSetField(image, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISWHITE);
    //    TIFFSetField(image, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISBLACK);
    //    TIFFSetField(image, TIFFTAG_FILLORDER, FILLORDER_MSB2LSB);
    //    TIFFSetField(image, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
    //    //  TIFFSetField(image, TIFFTAG_XRESOLUTION, 30.0);
    //    //  TIFFSetField(image, TIFFTAG_YRESOLUTION, 30.0);
    //    //  TIFFSetField(image, TIFFTAG_RESOLUTIONUNIT, RESUNIT_INCH);

    //    // Write the information to the file
    //    TIFFWriteEncodedStrip(image, 0, (char*)rawBuff[0], 8304 * 6220 *2);
    //    //    for (j = 0; j < 144; j++)
    //    //        TIFFWriteScanline(image, &buffer[j * 25], j, 0);
    //    // Close the file
    //    TIFFClose(image);
    FILE* fp = fopen("output_full.tif", "wb");
    fwrite((char*)tifBuff[0], 1, 8304*6220*2+8+138, fp);
    fclose(fp);
    QMessageBox box;
    box.setText(tr("done %1")
                .arg( QTime::currentTime().msecsSinceStartOfDay() - c1.msecsSinceStartOfDay() ));
    box.exec();
}

void MainWindow::on_actionNetCamera_triggered()
{
    SettingDialog sd;
    sd.exec();
}

void MainWindow::on_actionTiffToBitmap_triggered()
{
    QString filename = QFileDialog::getOpenFileName(this,
                                                    tr("Convert Tiff file to Bitmap"),
                                                    tr(""),
                                                    tr("TIFF Images (*.tif *.tiff)"));
    if(filename.isEmpty())
        return;
    time_t t1, t2;
    time(&t1);
    tiff_to_bmp(filename.toStdString().c_str());
    time(&t2);
    QMessageBox box(this);
    box.setText(tr("done %1")
                .arg( difftime(t2,t1), 0, 'g', 6 ));
    box.exec();
}

void MainWindow::on_actionPackageCount_triggered()
{
    QMessageBox box;
    box.setText(tr("package counter is %1\n").arg(packageCounter[0])
            );
    box.exec();
}

void MainWindow::on_actionBatchTiffToBitmap_triggered()
{
    QStringList files = QFileDialog::getOpenFileNames(this,
                                                      tr("Batch convert Tiff file to Bitmap"),
                                                      tr(""),
                                                      tr("TIFF Images (*.tif *.tiff)"));
      if(files.isEmpty())
          return;
    time_t t1,t2;
    time(&t1);
    for(int i=0; i< files.size(); i++)
    {
        tiff_to_bmp(files.at(i).toStdString().c_str() );
    }
    time(&t2);
    QMessageBox box(this);
    box.setText(tr("done %1")
                .arg( difftime(t2,t1) ));
    box.exec();
}
