#ifndef IMAGEHANDLETHREAD_H
#define IMAGEHANDLETHREAD_H
#include <QRunnable>
#include <QObject>
#include <QMutex>
#include "../ImageHandle/imagehandle.h"
class ImageHandleThread : public QObject,public QRunnable
{
    Q_OBJECT
signals:
    void SendImage(Mat Image);
private:
    QMutex ImageHandleLock;
    ImageHandle *mImageHandle;
    bool isRun=true;
public:
    ImageHandleThread();
    void run() override;
    void Stop();
public slots:
    void SetImageHandle(ImageHandle *handle);
};

#endif // IMAGEHANDLETHREAD_H
