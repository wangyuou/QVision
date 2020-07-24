#include "blobimagehandle.h"
#include <QMutex>
#include <string>
#include <vector>
#include <QTime>
#include <QDebug>
using namespace std;
using namespace cv;
//类中静态变量必须在类外面初始化
BLOBImageHandle * BLOBImageHandle::mInstance = NULL;
BLOBImageHandle::BLOBImageHandle()
{

}
BLOBImageHandle* BLOBImageHandle::GetInstance()
{
    static QMutex mLock;
    //单例双重校验
    if (mInstance == NULL)
    {
        mLock.lock();
        {
            if (mInstance == NULL) {
                mInstance = new BLOBImageHandle;
            }
        }
        mLock.unlock();
    }
    return mInstance;
}

Mat BLOBImageHandle::Handle(Mat image)
{
    //预处理
    if(Filter == 1)
    {
        //中值滤波
        cv::medianBlur(image,image,FilterSize);
    }
    else if(Filter == 2)
    {
        //均值滤波
        blur(image,image,Size(FilterSize,FilterSize));
    }
    else if(Filter == 3)
    {
        //高斯滤波
        GaussianBlur(image,image,Size(FilterSize,FilterSize),0,0);
    }

    QTime time;
    time.start();
    Ptr<Feature2D> b;
    b = SimpleBlobDetector::create(pDefaultBLOB);
    // We can detect keypoint with detect method
    vector<KeyPoint>  keyImg;
    Mat result(image.rows, image.cols, CV_8UC3);
    if (b.dynamicCast<SimpleBlobDetector>().get())
    {
        Ptr<SimpleBlobDetector> sbd = b.dynamicCast<SimpleBlobDetector>();
        sbd->detect(image, keyImg,Mat());
        qDebug()<<"BLOB:"<<time.elapsed()<<endl;
        drawKeypoints(image, keyImg, result);
        qDebug()<<"Draw:"<<time.elapsed()<<endl;
        QList<QPoint> points;
        QList<float> angles;
        for (vector<KeyPoint>::iterator k = keyImg.begin(); k != keyImg.end(); ++k)
        {            
            Point2f point = k->pt;
            Point2f pointStart;
            pointStart.x = point.x-(k->size/2);
            pointStart.y = point.y-(k->size/2);
            Point2f pointEnd;
            pointEnd.x = point.x+(k->size/2);
            pointEnd.y = point.y+(k->size/2);
            rectangle(result,pointStart,pointEnd,Scalar(255),3);
            points.append(QPoint(k->pt.x,k->pt.y));
            angles.append(k->angle);
        }
        emit SendCoordinate(points,angles);
        return result;
    }    
    return image;
}

BLOBImageHandle::~BLOBImageHandle()
{
    if(mInstance != NULL)
    {
        delete mInstance;
        mInstance = NULL;
    }
}

void BLOBImageHandle::SetParameter(int Filter,int FilterSize,SimpleBlobDetector::Params params)
{
    this->Filter = Filter;
    this->FilterSize = FilterSize;
    pDefaultBLOB = params;
}

