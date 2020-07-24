#include "Global.h"
CameraBasic *mCamera;
moodycamel::ConcurrentQueue<_Data> mImageQueue;
//队列长度
QSemaphore mTotalNum(5);
//可获得的队列长度
QSemaphore mAvailableNum(0);
bool mIsOpen=false;
