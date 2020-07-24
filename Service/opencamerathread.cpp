#include "opencamerathread.h"
#include "common/Global.h"
OpenCameraThread::OpenCameraThread()
{

}
void OpenCameraThread::run()
{
    emit SendOpenCameraState(mCamera->OpenCamera(index));
}
void OpenCameraThread::SetIndex(int index)
{
    this->index = index;
}
