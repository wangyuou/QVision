#ifndef GLOBAL_H
#define GLOBAL_H
#include "Camera/camerabasic.h"
#include "../ImageHandle/imagehandle.h"
#include "../common/concurrentqueue.h"
#include <QSemaphore>
#include <QTime>
typedef struct Data{
    Mat Image;
    QTime time;
}_Data;
//全局相机类
extern CameraBasic *mCamera;
extern moodycamel::ConcurrentQueue<_Data> mImageQueue;
extern QSemaphore mTotalNum;
extern QSemaphore mAvailableNum;
extern bool mIsOpen;
#endif // GLOBAL_H
