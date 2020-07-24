#include "utils.h"
#include <QDebug>
#include <opencv2/opencv.hpp>

Utils::Utils()
{

}

QImage Utils::cvMat_to_QImage(const Mat &mat) {
  switch ( mat.type() )
  {
     // 8-bit, 4 channel
     case CV_8UC4:
     {
        QImage image( mat.data, mat.cols, mat.rows, mat.step, QImage::Format_RGB32 );
        return image;
     }

     // 8-bit, 3 channel
     case CV_8UC3:
     {
        QImage image( mat.data, mat.cols, mat.rows, mat.step, QImage::Format_RGB888 );
        return image.rgbSwapped();
     }

     // 8-bit, 1 channel
     case CV_8UC1:
     {
        static QVector<QRgb>  sColorTable;
        // only create our color table once
        if ( sColorTable.isEmpty() )
        {
           for ( int i = 0; i < 256; ++i )
              sColorTable.push_back( qRgb( i, i, i ) );
        }
        QImage image( mat.data, mat.cols, mat.rows, mat.step, QImage::Format_Indexed8 );
        image.setColorTable( sColorTable );
        return image;
     }

     default:
        qDebug("Image format is not supported: depth=%d and %d channels\n", mat.depth(), mat.channels());
        break;
  }
  return QImage();
}

Mat Utils::QImage_to_cvMat( const QImage &image, bool inCloneImageData) {
  switch ( image.format() )
  {
     // 8-bit, 4 channel
     case QImage::Format_RGB32:
     {
        cv::Mat mat( image.height(), image.width(), CV_8UC4, const_cast<uchar*>(image.bits()), image.bytesPerLine() );
        return (inCloneImageData ? mat.clone() : mat);
     }

     // 8-bit, 3 channel
     case QImage::Format_RGB888:
     {
        if ( !inCloneImageData ) {
           qWarning() << "ASM::QImageToCvMat() - Conversion requires cloning since we use a temporary QImage";
        }
        QImage swapped = image.rgbSwapped();
        return cv::Mat( swapped.height(), swapped.width(), CV_8UC3, const_cast<uchar*>(swapped.bits()), swapped.bytesPerLine() ).clone();
     }

     // 8-bit, 1 channel
     case QImage::Format_Indexed8:
     {
        cv::Mat  mat( image.height(), image.width(), CV_8UC1, const_cast<uchar*>(image.bits()), image.bytesPerLine() );

        return (inCloneImageData ? mat.clone() : mat);
     }

     default:
        qDebug("Image format is not supported: depth=%d and %d format\n", image.depth(), image.format());
        break;
  }

  return cv::Mat();
}

// convert data stream in Mat format
Mat Utils::Convert2Mat(MV_FRAME_OUT_INFO_EX* pstImageInfo, unsigned char * pData,bool isGray)
{
    Mat srcImage;
    //printf("pstImageInfo->enPixelType:%d\n",pstImageInfo->enPixelType);
    if ( pstImageInfo->enPixelType == PixelType_Gvsp_Mono8 )
    {
        srcImage = cv::Mat(pstImageInfo->nHeight, pstImageInfo->nWidth,CV_8UC1, pData);
    }
    else if ( pstImageInfo->enPixelType == PixelType_Gvsp_BayerRG8 )
    {
        srcImage = cv::Mat(pstImageInfo->nHeight,pstImageInfo->nWidth,CV_8UC1,pData);
        if(!isGray)
        {
            cvtColor(srcImage,srcImage,COLOR_BayerBG2BGR);
        }
    }
    else
    {
        printf("unsupported pixel format\n");
        return srcImage;
    }

    if ( NULL == srcImage.data )
    {
        return srcImage;
    }
    return srcImage;
}

/************************************************************************
*函数名：        get_point_angle
*
*函数作用：      已知2个坐标点，求从 0------->x 逆时针需旋转多少角度到该位置
*
*                   |
*                   |
*                   |
*                   |
*------------------------------------> x
*                   | 0
*                   |
*                   |
*                   |
*                   v
*                   y
*
*函数参数：
*CvPoint2D32f pointO  - 起点
*CvPoint2D32f pointA  - 终点
*
*函数返回值：
*double         向量OA，从 0------->x 逆时针需旋转多少角度到该位置
**************************************************************************/
double Utils::get_point_angle(Point2f pointO,Point2f pointA)
{
    double angle = 0;
    Point2f point;
    double temp;

    point = Point2f((pointA.x - pointO.x), (pointA.y - pointO.y));

    if (fabs(point.x) <= 1e-6 && fabs(point.y) <= 1e-6)
    {
        return 0;
    }

    if (fabs(point.x) <= 1e-6)
    {
        angle = 90;
        return angle;
    }

    if (fabs(point.y) <= 1e-6)
    {
        angle = 0;
        return angle;
    }

    temp = fabsf(float(point.y)/float(point.x));
    temp = atan(temp);
    temp = temp*180/CV_PI ;
    if ((0<point.x)&&(0<point.y))
    {
        angle = 360 - temp;
        return angle;
    }

    if ((0>point.x)&&(0<point.y))
    {
        angle = 180+ temp;
        return angle;
    }

    if ((0<point.x)&&(0>point.y))
    {
        angle = temp;
        return angle;
    }

    if ((0>point.x)&&(0>point.y))
    {
        angle = 180 - temp;
        return angle;
    }

    //printf("sceneDrawing :: getAngle error!");
    return temp;
}

