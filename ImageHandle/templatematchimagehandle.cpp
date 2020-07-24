#include "templatematchimagehandle.h"
#include <opencv2\imgproc\types_c.h>
#include <QMutex>
#include <QTime>
#include <QDebug>
#include "./Common/utils.h"
#include "./TemplateMatch/Interface.h"
TemplateMatchImageHandle* TemplateMatchImageHandle::mInstance = NULL;
TemplateMatchImageHandle::TemplateMatchImageHandle()
{
}

TemplateMatchImageHandle::~TemplateMatchImageHandle()
{
    if(mInstance == NULL)
    {
        delete mInstance;
        mInstance = NULL;
    }
}

TemplateMatchImageHandle* TemplateMatchImageHandle::GetInstance()
{
    static QMutex mLock;
    //单例双重校验
    if (mInstance == NULL)
    {
        mLock.lock();
        {
            if (mInstance == NULL) {
                mInstance = new TemplateMatchImageHandle;
            }
        }
        mLock.unlock();
    }
    return mInstance;
}

Mat TemplateMatchImageHandle::Handle(Mat image)
{
    QTime time;
    time.start();
    if(image.empty())
    {
        return image;
    }
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
    if(image.channels() != 1)
    {
        cvtColor(image,image,COLOR_BayerBG2GRAY);
    }
    if(TemplateMode == 0)
    {
        //轮廓匹配
        threshold(image, image, minThreshold, maxThreshold, CV_THRESH_BINARY);
        qDebug()<<ShapeItem<<":"<<mSize<<endl;
        //形态学
        if(ShapeItem != 0)
        {
            Mat kernel = getStructuringElement(MORPH_RECT,Size(this->mSize,this->mSize));//创建结构元素大小为3*3
            morphologyEx(image,image,ShapeItem-1,kernel);
            kernel.release();
        }
        vector<vector<Point>> contours2;
        vector<Vec4i> hierarcy2;
        QList<QPoint> points;
        QList<float> angles;
        findContours(image, contours2, hierarcy2, CV_RETR_CCOMP, CV_CHAIN_APPROX_NONE);
        for (unsigned int i = 0; i<contours2.size(); i++)
        {
            double matchRate = matchShapes(contours[0], contours2[i], CV_CONTOURS_MATCH_I1, 0.0);//形状匹配:值越小越相似
            if (matchRate <= result)
            {
                Rect rect = boundingRect(Mat(contours2[i]));
                RotatedRect boundRect = minAreaRect(contours2[i]);
                rectangle(image,rect.tl(), rect.br(), 255, 2, 8, 0);
                points.append(QPoint(boundRect.center.x,boundRect.center.y));
                angles.append(boundRect.angle-StartAngle);
            }
        }
        emit SendCoordinate(points,angles);
        qDebug()<<"模板匹配时间:"<<time.elapsed()<<endl;
        return image;
    }
    else
    {
        angle_test(image,NULL,"test",true);
        return image;
    }

}

void TemplateMatchImageHandle::SetParameter(int Filter,int FilterSize,int minThreshold,int maxThreshold,vector<vector<Point>> contours,int ShapeItem,int mSize,double result,float StartAngle,int TemplateMode)
{
    this->Filter = Filter;
    this->FilterSize = FilterSize;
    this->minThreshold = minThreshold;
    this->maxThreshold = maxThreshold;
    this->contours = contours;
    this->result = result;
    this->ShapeItem = ShapeItem;
    this->mSize = mSize;
    this->StartAngle = StartAngle;
    this->TemplateMode = TemplateMode;
}

