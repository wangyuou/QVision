#ifndef UTILS_H
#define UTILS_H
#include "opencv2/highgui/highgui.hpp"
using namespace cv;
#include "MvCameraControl.h"
#include <QImage>
class Utils{
private:
    //不允许实例化
    Utils();
public:
    static QImage cvMat_to_QImage(const Mat &mat);
    static Mat QImage_to_cvMat( const QImage &image, bool inCloneImageData=false);
    static Mat Convert2Mat(MV_FRAME_OUT_INFO_EX* pstImageInfo, unsigned char * pData,bool isGray);
    static double get_point_angle(Point2f pointO,Point2f pointA);
};
#endif // UTILS_H
