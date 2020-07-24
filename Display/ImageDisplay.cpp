#include "ImageDisplay.h"
#include <qDebug>
#include "../common/utils.h"
#include <QPixmap>
#include <vector>
#include <algorithm>
#include "./Common/Global.h"
using namespace std;
ImageDisplay::ImageDisplay(QWidget* parent):
    QLabel(parent)

{

}
void ImageDisplay:: mouseReleaseEvent(QMouseEvent *ev)
{
    if(mIsOpen)
    {
        return ;
    }
    if(isMove)
    {
        if(ev->button() == Qt::LeftButton)
        {
            isPress = false;
            isMove = false;
            //发送模板图片
            Rect rect(StartPoint.x(),StartPoint.y(),EndPoint.x()-StartPoint.x(),EndPoint.y()-StartPoint.y());
            if(isOnce)
            {
                 SaveClipImageList.append(TempImage.clone());
                 isOnce = false;
            }
            Mat image_roi = TempImage(rect);
            SaveClipImageList.append(image_roi.clone());
            emit SendRegion(image_roi);
        }
    }
    else
    {
        if(ev->button() == Qt::RightButton)
        {
            if(SaveClipImageList.size()>0)
            {
                SaveClipImageList.pop_back();
                emit SendRegion(SaveClipImageList.at(SaveClipImageList.size()-1));
            }
            else
            {
                isOnce = true;
            }
        }
        else if(ev->button() == Qt::LeftButton)
        {
            //获取HSV值
            if(CurrentState == 0)
            {
                //获取图像
                const QPixmap *pix = pixmap();
                if(pix == NULL)
                {
                    return ;
                }
                Mat image = Utils::QImage_to_cvMat(pix->toImage(),true);
                if(pix == nullptr || image.channels() == 1)
                {
                    return ;
                }
                int PosX,PosY;
                PosX = ev->pos().x();
                PosY = ev->pos().y();
                qDebug()<<"X:"<<PosX<<"Y:"<<PosY<<"H:"<<height()<<"W:"<<width()<<endl;
                int mWidth = image.cols;
                int mHeight = image.rows;
                int SpaceRows = (height()-mHeight)/2;
                int SpaceCols = (width()-mWidth)/2;
                //将图像转成HSV
                cvtColor(image, image, COLOR_BGR2HSV);
                if((PosX>SpaceCols) && (PosX<SpaceCols+mWidth) && (PosY>SpaceRows) && (PosY<SpaceRows+mHeight))
                {
                    cv::Point point(PosX-SpaceCols,PosY-SpaceRows);
                    int H = static_cast<int>( image.at<Vec3b>(point)[0]);
                    int S = static_cast<int>( image.at<Vec3b>(point)[1]);
                    int V = static_cast<int>( image.at<Vec3b>(point)[2]);
                    emit SendHSV(H,S,V);
                }
                image.release();
            }
            //模板匹配
            else if(CurrentState == 2)
            {
                //选取连通域
                const QPixmap *pix = pixmap();
                Mat image = Utils::QImage_to_cvMat(pix->toImage(),true);
                if(pix == nullptr || image.channels() == 1)
                {
                    return ;
                }
                cvtColor(image, image, COLOR_BGR2GRAY);
                Mat  img_edge, labels, centroids, img_color, stats;
                threshold(image, img_edge, 0, 255, THRESH_OTSU);
                int nccomps = connectedComponentsWithStats(img_edge, labels, stats, centroids);
                //获取点击点的标签
                int PosX,PosY;
                PosX = ev->pos().x();
                PosY = ev->pos().y();
                int mWidth = image.cols;
                int mHeight = image.rows;
                int SpaceRows = (height()-mHeight)/2;
                int SpaceCols = (width()-mWidth)/2;
                int lab = labels.at<int>(PosY-SpaceRows, PosX-SpaceCols);
                Vec3b color= Vec3b(rand() % 256, rand() % 256, rand() % 256);
                img_color = Mat::zeros(image.size(), CV_8UC3);
                for (int y = 0; y < img_color.rows; y++)
                {
                    for (int x = 0; x < img_color.cols; x++)
                    {
                        int label = labels.at<int>(y, x);
                        //qDebug()<<"label:"<<label;
                        CV_Assert(0 <= label && label <= nccomps);
                        if(lab == label)
                        {
                            img_color.at<Vec3b>(y, x) = color;
                        }
                    }
                }
                setPixmap(QPixmap::fromImage(Utils::cvMat_to_QImage(img_color)));
                //发送到ThresholdImage
                emit SendThresholdImage(img_color);
            }
        }
    }
}

void ImageDisplay::mousePressEvent(QMouseEvent *ev)
{
    if(mIsOpen)
    {
        return ;
    }
    if(ev->button() == Qt::LeftButton)
    {
        //获取图像
        const QPixmap *pix = pixmap();
        if(pix == NULL)
        {
            return ;
        }
        TempImage = Utils::QImage_to_cvMat(pix->toImage(),true);        
        int PosX,PosY;
        PosX = ev->pos().x();
        PosY = ev->pos().y();
        int mWidth = TempImage.cols;
        int mHeight = TempImage.rows;
        int SpaceRows = (height()-mHeight)/2;
        int SpaceCols = (width()-mWidth)/2;
        if((PosX>SpaceCols) && (PosX<SpaceCols+mWidth) && (PosY>SpaceRows) && (PosY<SpaceRows+mHeight))
        {
            StartPoint.setX(ev->pos().x()-SpaceCols);
            StartPoint.setY(ev->pos().y()-SpaceRows);
            isPress = true;
        }
    }
}

void ImageDisplay::mouseMoveEvent(QMouseEvent *ev)
{
    if(mIsOpen)
    {
        return ;
    }
    if(isPress)
    {
        int PosX,PosY;
        PosX = ev->pos().x();
        PosY = ev->pos().y();
        int mWidth = TempImage.cols;
        int mHeight = TempImage.rows;
        int SpaceRows = (height()-mHeight)/2;
        int SpaceCols = (width()-mWidth)/2;
        EndPoint.setX(PosX-SpaceCols);
        EndPoint.setY(PosY-SpaceRows);
        if(EndPoint.x()>TempImage.cols)
        {
            EndPoint.setX(TempImage.cols);
        }
        if(EndPoint.y()>TempImage.rows)
        {
            EndPoint.setY(TempImage.rows);
        }
        Mat Image;
        TempImage.copyTo(Image);
        Rect rect(StartPoint.x(),StartPoint.y(),EndPoint.x()-StartPoint.x(),EndPoint.y()-StartPoint.y());
        rectangle(Image,rect,Scalar(100),2);
        emit SendRegion(Image);
        isMove = true;
    }
}

void ImageDisplay::TabWidgetChange(int state)
{
    CurrentState = state;    
}

void ImageDisplay::ClearClipImageList()
{
    isOnce=true;
    SaveClipImageList.clear();
}
