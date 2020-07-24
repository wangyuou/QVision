#ifndef TEMPLATEMATCHIMAGEHANDLE_H
#define TEMPLATEMATCHIMAGEHANDLE_H
#include "imagehandle.h"
#include <vector>
#include <QList>
#include <QPoint>
#include "opencv2/features2d.hpp"
#include "opencv2/xfeatures2d.hpp"
using namespace std;
using namespace cv;
using namespace cv::xfeatures2d;
class TemplateMatchImageHandle : public ImageHandle
{
    Q_OBJECT
signals:
    void SendCoordinate(QList<QPoint>,QList<float>);
private:
    int Filter;
    int FilterSize;
    int minThreshold;
    int maxThreshold;
    vector<vector<Point>> contours;
    double result;
    int ShapeItem;
    int mSize;
    float StartAngle;
    int TemplateMode;
public:
    TemplateMatchImageHandle();
    ~TemplateMatchImageHandle();
    static TemplateMatchImageHandle *mInstance;
    static TemplateMatchImageHandle* GetInstance();
    Mat Handle(Mat image);
    void SetParameter(int Filter,int FilterSize,int minThreshold,int maxThreshold,vector<vector<Point>> contours,\
                      int ShapeItem,int mSize,double result,float StartAngle,int TemplateMode);
};
#endif // TEMPLATEMATCHIMAGEHANDLE_H
