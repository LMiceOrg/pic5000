#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "camerathread.h"
#include "udp_pic5000.h"

QT_BEGIN_NAMESPACE
class QGraphicsScene;
class QSplitter;
QT_END_NAMESPACE

namespace Ui {
class MainWindow;
}

extern struct timeval totalStartTime[NETCAP_CHANNEL_COUNT];
extern struct timeval totalEndTime[NETCAP_CHANNEL_COUNT];

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
protected:

signals:
    void currentFileName(QString name);
protected slots:
    void toggleViewMaximize(int i);
    void toggleViewNormalize(int i);
    void resetScene(int index);
    void saveSceneToTiff(int index,const QString name);
    void changeToRGB(int index);
    void startSnapPicture(int index);
private slots:
    void on_pushButton_clicked();

    void on_pushButton_2_clicked();

    void on_pushButton_3_clicked();

    void on_pushButton_4_clicked();

    void on_pushButton_5_clicked();

    void on_actionNetCamera_triggered();

    void on_actionTiffToBitmap_triggered();

    void on_actionPackageCount_triggered();

    void on_actionBatchTiffToBitmap_triggered();

private:
    Ui::MainWindow *ui;
    QGraphicsScene *scene[NETCAP_CHANNEL_COUNT];
    QSplitter *h1Splitter;
    QSplitter *h2Splitter;
    QSplitter *vSplitter;
    QByteArray spliterStates[3];

};

#endif // MAINWINDOW_H
