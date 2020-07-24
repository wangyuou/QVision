#include "hsvimagehandle.h"
#include <QMutex>
#include <opencv2/tracking.hpp>
#include <QDebug>
#include <vector>
#include <QTime>
HSVImageHandle* HSVImageHandle::mInstance = NULL;
HSVImageHandle::HSVImageHandle(int Filter,int FilterSize,int HMin,int HMax,int SMin,int SMax,int VMin,int VMax,int ShapeItem,int Size)\
    :HMin(HMin),HMax(HMax),SMin(SMin),SMax(SMax),VMin(VMin),VMax(VMax),ShapeItem(ShapeItem),mSize(Size),Filter(Filter),FilterSize(FilterSize)
{

}

HSVImageHandle::~HSVImageHandle()
{
    if(mInstance == NULL)
    {
        delete mInstance;
        mInstance = NULL;
    }
}

Mat HSVImageHandle::Handle(Mat image)
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
    cvtColor(image, image, COLOR_BGR2HSV);
    IplImage tmp=IplImage(image);//添加的代码
    IplImage *mask;
    CvArr* arr = (CvArr*)&tmp;//添加的代码
    mask=cvCreateImage(cvGetSize(arr),8,1);
    cvInRangeS(arr,Scalar(qMin(HMin,HMax),qMin(SMin,SMax),qMin(VMin,VMax),0),\
               Scalar(qMax(HMin,HMax),qMax(SMin,SMax),qMax(VMin,VMax),0),mask);
    image = cv::cvarrToMat(mask).clone();
    cvReleaseImage(&mask);
    if(ShapeItem != 0)
    {
        //形态学
        Mat kernel = getStructuringElement(MORPH_RECT,Size(this->mSize,this->mSize));//创建结构元素大小为3*3
        morphologyEx(image,image,ShapeItem-1,kernel);
        kernel.release();
    }
    qDebug()<<"HSV1:"<<time.elapsed()<<endl;
    Mat  img_edge, labels, centroids, stats;
    Mat  *img_color;
    threshold(image, img_edge, 0, 255, THRESH_OTSU);
    int nccomps = connectedComponentsWithStats(img_edge, labels, stats, centroids);
    if(nccomps > 100)
    {
        qDebug()<<"选择数据太多!"<<endl;
        return image;
    }
    try{
        img_color = new Mat[nccomps];
    }catch(std::bad_alloc)
    {
        qDebug()<<"分配空间失败!"<<endl;
        return image;
    }

    QList<int> SmallerArea;
    for(int i = 0;i<nccomps;++i)
    {       
        img_color[i].create(image.size(),CV_8UC1);
        Mat mat = Mat::zeros(image.size(),CV_8UC1);
        mat.copyTo(img_color[i]);
        if (stats.at<int>(i, CC_STAT_AREA) > Area)
        {
            SmallerArea.append(i);
        }
    }
    for (int y = 0; y < image.rows; y++)
    {
        for (int x = 0; x < image.cols; x++)
        {
            int label = labels.at<int>(y, x);
            if(!SmallerArea.contains(label))
            {
                continue;
            }
            CV_Assert(0 <= label && label <= nccomps);
            uchar* ptr = img_color[label].ptr<uchar>(y);
            ptr[x] = 255;  //赋值或取值
        }
    }
    QList<QPoint> point;
    QList<float> angle;
    foreach (int i, SmallerArea)
    {        
        std::vector<std::vector<Point> > contours;
        std::vector<Vec4i> hierarchy;
        findContours(img_color[i], contours, hierarchy, RETR_CCOMP, CHAIN_APPROX_SIMPLE);
        RotatedRect rect = minAreaRect(contours[0]);
        if(rect.boundingRect().area()<Area)
        {
            continue;
        }
        rectangle(image,rect.boundingRect().br(),rect.boundingRect().tl(),Scalar(255),3);
        point.append(QPoint(rect.center.x,rect.center.y));
        angle.append(rect.angle);
    }
    delete []img_color;
    qDebug()<<"HSV:"<<time.elapsed()<<endl;
    emit SendCoordinate(point,angle);    
    return image;
}
HSVImageHandle* HSVImageHandle::GetInstance()
{
    static QMutex mLock;
    //单例双重校验
    if (mInstance == NULL)
    {
        mLock.lock();
        {
            if (mInstance == NULL) {
                mInstance = new HSVImageHandle;
            }
        }
        mLock.unlock();
    }
    return mInstance;
}
void HSVImageHandle::SetParameter(int Filter,int FilterSize,int HMin,int HMax,int SMin,int SMax,int VMin,int VMax,int ShapeItem,int mSize,int Area)
{
    this->HMin = HMin;
    this->HMax = HMax;
    this->SMin = SMin;
    this->SMax = SMax;
    this->VMin = VMin;
    this->VMax = VMax;
    this->ShapeItem = ShapeItem;
    this->mSize = mSize;
    this->Filter = Filter;
    this->FilterSize = FilterSize;
    this->Area = Area;
}

void HSVImageHandle::SetArea(int area)
{
    Area = area;
}
