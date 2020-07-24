#ifndef HAIKANGCAMERA_H
#define HAIKANGCAMERA_H
#include "camerabasic.h"
#include "MvCameraControl.h"
#include <QMutex>
#include <QThreadPool>
#include <QRunnable>
#include "../ImageHandle/imagehandle.h"
class HaiKangCamera:public CameraBasic,public QRunnable
{
    Q_OBJECT
private:
    QMutex DeviceListLock;
    QMutex isGrayLock;    
    MV_CC_DEVICE_INFO_LIST stDeviceList;
    void* handle = NULL;    
    unsigned int g_nPayloadSize = 0;
    int nRet = MV_OK;
    MV_CC_DEVICE_INFO_LIST NewDeviceList;
    bool isGray=false;    
public:
    HaiKangCamera();
    ~HaiKangCamera();
    bool OpenCamera(int index);
    bool CloseCamera();
    QList<QString>EnumDeviceInfo();
    void run() override;
    void DestroyHandle();
public slots:
    void SetGrayImage(int state);
};

#endif // HAIKANGCAMERA_H
