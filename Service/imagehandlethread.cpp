#include "imagehandlethread.h"
#include "../common/Global.h"
#include <QDebug>
#include <QThread>
#include <QTime>
ImageHandleThread::ImageHandleThread()
{

}
void ImageHandleThread::run()
{
    int ii = 0;
    while(isRun)
    {
        mAvailableNum.acquire(1);
        _Data item;
        bool ret = mImageQueue.try_dequeue(item);
        if(ret)
        {
            qDebug()<<":receive:"<<item.time.toString("hh:mm:ss.zzz")<<"->"<<QTime::currentTime().toString("hh:mm:ss.zzz")<<endl;
            //进行图像的处理
            QMutexLocker locker(&ImageHandleLock);
            if(mImageHandle != NULL)
            {
                qDebug()<<"!=NULL"<<endl;
                item.Image = mImageHandle->Handle(item.Image);
            }
            else
            {
                qDebug()<<"NULL"<<endl;
            }
            emit(SendImage(item.Image.clone()));
            item.Image.release();
        }
        else
        {
            qDebug()<<"获取图像失败!!!"<<endl;
        }
        mTotalNum.release(1);
    }
}

void ImageHandleThread::SetImageHandle(ImageHandle *handle)
{
    QMutexLocker locker(&ImageHandleLock);
    mImageHandle = handle;
}

void ImageHandleThread::Stop()
{
    isRun = false;
}
