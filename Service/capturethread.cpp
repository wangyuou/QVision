#include "capturethread.h"
#include "opencv2/highgui/highgui.hpp"
using namespace cv;
#include "Common/utils.h"
#include <QDebug>
#include <QTime>
CaptureThread::CaptureThread()
{
    mImageHandle = NULL;
}

CaptureThread::~CaptureThread()
{
    if(mImageHandle != NULL)
    {
        delete mImageHandle;
        mImageHandle = NULL;
    }
}

void CaptureThread::run()
{
    QTime time;
    time.start();
    int nRet = MV_OK;
    MV_FRAME_OUT stOutFrame = {0};
    memset(&stOutFrame, 0, sizeof(MV_FRAME_OUT));
    //qDebug()<<"thread:"<<(*isopen)<<handle<<endl;
    while(*isopen)
    {
        nRet = MV_CC_GetImageBuffer(*handle, &stOutFrame, 1000);
        if (nRet == MV_OK)
        {
            mLock.lock();
            Mat Image = Utils::Convert2Mat(&stOutFrame.stFrameInfo,stOutFrame.pBufAddr,isGray);
            mLock.unlock();
            //对图像进行处理
            if(mImageHandle != NULL)
            {
                Image = mImageHandle->Handle(Image);
            }
            emit(SendImage(Image.clone()));
            //Image.release();
        }
        else
        {
            printf("No data[0x%x]\n", nRet);
        }
        if(NULL != stOutFrame.pBufAddr)
        {
            nRet = MV_CC_FreeImageBuffer(*handle, &stOutFrame);
            if(nRet != MV_OK)
            {
                printf("Free Image Buffer fail! nRet [0x%x]\n", nRet);
            }
        }
    }
    qDebug()<<"time:"<<time.elapsed()<<endl;
}
void CaptureThread::SetImageHandle(ImageHandle *handle)
{
    mImageHandle = handle;
}

ImageHandle* CaptureThread::GetImageHandle()
{
    return mImageHandle;
}

void CaptureThread::SetGrayImage(int state)
{
    if(mImageHandle != NULL)
    {
        return ;
    }
    mLock.lock();
    if(state != 2)
    {
        this->isGray = false;
    }
    else
    {
        this->isGray = true;
    }
    mLock.unlock();
}
