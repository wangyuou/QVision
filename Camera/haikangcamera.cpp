#include "haikangcamera.h"
#include <QMutexLocker>
#include <QDebug>
#include <QTime>
#include "../common/utils.h"
#include "../common/Global.h"
HaiKangCamera::HaiKangCamera()
{
    setAutoDelete(false);
}
HaiKangCamera::~HaiKangCamera()
{
    DestroyHandle();
}
bool HaiKangCamera::OpenCamera(int index)
{
    //相机已经打开
    if(mIsOpen)
    {
        return false;
    }
    QMutexLocker locker(&DeviceListLock);
    if (index >= (signed int)(stDeviceList.nDeviceNum))
    {
        //QMessageBox::warning(NULL, "提示", "设备索引错误!", "确定");
        mIsOpen = false;
        return mIsOpen;
    }

    // ch:选择设备并创建句柄 | en:Select device and create handle
    nRet = MV_CC_CreateHandle(&handle, stDeviceList.pDeviceInfo[index]);
    if (MV_OK != nRet)
    {
        //QMessageBox::warning(NULL, "提示", "创建设备句柄错误!", "确定");
        qDebug("Create Handle fail! nRet [0x%x]\n",nRet);
        CloseCamera();
        DestroyHandle();
        mIsOpen = false;
        //强制刷新
        stDeviceList.nDeviceNum = 0;
        return mIsOpen;
    }
    // ch:打开设备 | en:Open device
    nRet = MV_CC_OpenDevice(handle);
    if (MV_OK != nRet)
    {
        qDebug()<<handle<<endl;
        //QMessageBox::warning(NULL, "提示", "打开设备失败!", "确定");
        qDebug("Open Device fail! nRet [0x%x]\n", nRet);
        CloseCamera();
        DestroyHandle();
        mIsOpen = false;
        //强制刷新
        stDeviceList.nDeviceNum = 0;
        return false;
    }

    // ch:探测网络最佳包大小(只对GigE相机有效) | en:Detection network optimal package size(It only works for the GigE camera)
    if (stDeviceList.pDeviceInfo[index]->nTLayerType == MV_GIGE_DEVICE)
    {
        int nPacketSize = MV_CC_GetOptimalPacketSize(handle);
        if (nPacketSize > 0)
        {
            nRet = MV_CC_SetIntValue(handle,"GevSCPSPacketSize",nPacketSize);
            if(nRet != MV_OK)
            {
                printf("Warning: Set Packet Size fail nRet [0x%x]!", nRet);
            }
        }
        else
        {
            printf("Warning: Get Packet Size fail nRet [0x%x]!", nPacketSize);
        }
    }
    // ch:设置触发模式为off | en:Set trigger mode as off
    nRet = MV_CC_SetEnumValue(handle, "TriggerMode", 0);
    if (MV_OK != nRet)
    {
        //QMessageBox::warning(NULL, "提示", "设置相机触发模式失败!", "确定");
        printf("Set Trigger Mode fail! nRet [0x%x]\n", nRet);
        CloseCamera();
        DestroyHandle();
        mIsOpen = false;
        //强制刷新
        stDeviceList.nDeviceNum = 0;
        return false;
    }

    // ch:获取数据包大小 | en:Get payload size
    MVCC_INTVALUE stParam;
    memset(&stParam, 0, sizeof(MVCC_INTVALUE));
    nRet = MV_CC_GetIntValue(handle, "PayloadSize", &stParam);
    if (MV_OK != nRet)
    {
        //QMessageBox::warning(NULL, "提示", "获取相机信息失败!", "确定");
        printf("Get PayloadSize fail! nRet [0x%x]\n", nRet);
        CloseCamera();
        DestroyHandle();
        //强制刷新
        stDeviceList.nDeviceNum = 0;
        mIsOpen = false;
        return false;
    }
    g_nPayloadSize = stParam.nCurValue;

    // ch:开始取流 | en:Start grab image
    nRet = MV_CC_StartGrabbing(handle);
    if (MV_OK != nRet)
    {
        //QMessageBox::warning(NULL, "提示", "抓取图像失败!", "确定");
        printf("Start Grabbing fail! nRet [0x%x]\n", nRet);
        CloseCamera();
        DestroyHandle();
        //强制刷新
        stDeviceList.nDeviceNum = 0;
        mIsOpen = false;
        return false;
    }
    mIsOpen = true;
    //启动获取照片线程
    QThreadPool::globalInstance()->start(this);
    return mIsOpen;
}
bool HaiKangCamera::CloseCamera()
{
    //QMutexLocker locker(&DeviceListLock);
    bool Ret = true;
    //相机未打开
    if(!mIsOpen)
    {
        return false;
    }
    mIsOpen = false;
    // ch:停止取流 | en:Stop grab image
    nRet = MV_CC_StopGrabbing(handle);
    if (MV_OK != nRet)
    {
        printf("Stop Grabbing fail! nRet [0x%x]\n", nRet);
        Ret = false;
    }

    // ch:关闭设备 | Close device
    nRet = MV_CC_CloseDevice(handle);
    if (MV_OK != nRet)
    {
        printf("ClosDevice fail! nRet [0x%x]\n", nRet);
        Ret = false;
    }
//    // ch:销毁句柄 | Destroy handle
//    nRet = MV_CC_DestroyHandle(handle);
//    if (MV_OK != nRet)
//    {
//        printf("Destroy Handle fail! nRet [0x%x]\n", nRet);
//        Ret = false;
//    }
//    if (nRet != MV_OK)
//    {
//        if (handle != NULL)
//        {
//            MV_CC_DestroyHandle(handle);
//            handle = NULL;
//        }
//    }
    return Ret;
}
QList<QString> HaiKangCamera::EnumDeviceInfo()
{
    //qDebug()<<"device:"<<QThread::currentThreadId()<<endl;
    int nRet = MV_OK;
    QList<QString> list;
    // ch:枚举设备 | en:Enum device
    memset(&NewDeviceList, 0, sizeof(MV_CC_DEVICE_INFO_LIST));
    nRet = MV_CC_EnumDevices(MV_GIGE_DEVICE | MV_USB_DEVICE, &NewDeviceList);
    if (MV_OK != nRet)
    {
        printf("Enum Devices fail! nRet [0x%x]\n", nRet);
        //list.append(tr("错误码:"+nRet));
        //emit(SendDeviceInfo(list));
        return list;
    }

    if ((NewDeviceList).nDeviceNum > 0)
    {
        for (unsigned int i = 0; i < (NewDeviceList).nDeviceNum; i++)
        {
            list.append(QString("设备:")+QString::number(i));
        }
    }
    else
    {
        printf("Find No Devices!\n");
        //list.append(tr("没有找到设备!"));
    }
    if(NewDeviceList.nDeviceNum != stDeviceList.nDeviceNum)
    {
        QMutexLocker locker(&DeviceListLock);
        stDeviceList = NewDeviceList;
    }
    return list;
}
void HaiKangCamera::run()
{
    MV_FRAME_OUT stOutFrame = {0};
    memset(&stOutFrame, 0, sizeof(MV_FRAME_OUT));
    while(mIsOpen)
    {
        nRet = MV_CC_GetImageBuffer(handle, &stOutFrame, 1000);
        if (nRet == MV_OK)
        {
            _Data item;
            isGrayLock.lock();
            Mat Image = Utils::Convert2Mat(&stOutFrame.stFrameInfo,stOutFrame.pBufAddr,isGray);
            isGrayLock.unlock();
            //todo 换成生产者和消费者模型
            bool ret = mTotalNum.tryAcquire(1);
            if(!ret)
            {
                //如果接受端处理速度较慢，将自动丢帧                
                if(mImageQueue.try_dequeue(item))
                {
                    item.Image = Image.clone();
                    item.time = QTime::currentTime();
                    mImageQueue.enqueue(item);
                }
                if(NULL != stOutFrame.pBufAddr)
                {
                    nRet = MV_CC_FreeImageBuffer(handle, &stOutFrame);
                    if(nRet != MV_OK)
                    {
                        printf("Free Image Buffer fail! nRet [0x%x]\n", nRet);
                    }
                }
                Image.release();
                continue ;
            }
            item.Image = Image.clone();
            item.time = QTime::currentTime();
            mImageQueue.enqueue(item);
            mAvailableNum.release(1);
            Image.release();
        }
        else
        {
            printf("No data[0x%x]\n", nRet);
        }
        if(NULL != stOutFrame.pBufAddr)
        {
            nRet = MV_CC_FreeImageBuffer(handle, &stOutFrame);
            if(nRet != MV_OK)
            {
                printf("Free Image Buffer fail! nRet [0x%x]\n", nRet);
            }
        }
    }
    DestroyHandle();
}

void HaiKangCamera::SetGrayImage(int state)
{
    if(mIsOpen)
    {
        return;
    }
    QMutexLocker locker(&isGrayLock);
    if(state == 2)
    {
        isGray = true;
    }
    else
    {
        isGray = false;
    }
}

void HaiKangCamera::DestroyHandle()
{
    // ch:销毁句柄 | Destroy handle
    nRet = MV_CC_DestroyHandle(handle);
    if (MV_OK != nRet)
    {
        printf("Destroy Handle fail! nRet [0x%x]\n", nRet);
    }
    if (nRet != MV_OK)
    {
        if (handle != NULL)
        {
            MV_CC_DestroyHandle(handle);
            handle = NULL;
        }
    }
}
