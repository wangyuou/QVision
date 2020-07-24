#include "closecamerathread.h"
#include "common/Global.h"
CloseCameraThread::CloseCameraThread()
{

}
void CloseCameraThread::run()
{
    mCamera->CloseCamera();
}
