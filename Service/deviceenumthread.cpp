#include "deviceenumthread.h"
#include "common/Global.h"
#include <QList>
DeviceEnumThread::DeviceEnumThread()
{

}
void DeviceEnumThread::run()
{
    emit SendDeviceInfo(mCamera->EnumDeviceInfo());
}
