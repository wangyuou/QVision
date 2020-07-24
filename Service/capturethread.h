#ifndef CAPTURETHREAD_H
#define CAPTURETHREAD_H
#include <QRunnable>
#include <QObject>
#include <QDebug>
#include "MvCameraControl.h"
#include <opencv2/opencv.hpp>
#include "../ImageHandle/imagehandle.h"
#include <QMutex>
using namespace cv;
class CaptureThread : public QObject,public QRunnable
{
    Q_OBJECT
private:
    ImageHandle *mImageHandle;
    bool isGray = false;
    QMutex mLock;
public:
    void **handle;
    bool *isopen;
    void run() override;
    void SetImageHandle(ImageHandle *handle);
    ImageHandle* GetImageHandle();
    CaptureThread();
    ~CaptureThread();
signals:
    void SendImage(Mat image);
public slots:
    void SetGrayImage(int state);
};

#endif // CAPTURETHREAD_H
