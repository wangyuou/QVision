#ifndef OPENCAMERA_H
#define OPENCAMERA_H
#include <QObject>
#include <QRunnable>
class OpenCameraThread:public QObject,public QRunnable
{
    Q_OBJECT
signals:
    void SendOpenCameraState(bool);
private:
    int index;
public:
    OpenCameraThread();
    void run()override;
    void SetIndex(int index);
};

#endif // OPENCAMERA_H
