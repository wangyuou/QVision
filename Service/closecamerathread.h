#ifndef CLOSECAMERA_H
#define CLOSECAMERA_H
#include <QObject>
#include <QRunnable>
class CloseCameraThread:public QObject,public QRunnable
{
public:
    CloseCameraThread();
    void run()override;
};

#endif // CLOSECAMERA_H
