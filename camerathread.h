#ifndef CAMERATHREAD_H
#define CAMERATHREAD_H
#include <QThread>
class CameraThread:public QThread
{
    Q_OBJECT
    void run() Q_DECL_OVERRIDE ;
public:
    CameraThread(int index);
    void stopCamera();
private:
    int channel;
signals:
    void resultReady(const QString &s);
};

#endif // CAMERATHREAD_H
