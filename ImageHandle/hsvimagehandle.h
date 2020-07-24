#ifndef HSVIMAGEHANDLE_H
#define HSVIMAGEHANDLE_H
#include "imagehandle.h"
#include <QPoint>
#include <QList>
class HSVImageHandle : public ImageHandle
{
    Q_OBJECT
private:
    int HMin;
    int HMax;
    int SMin;
    int SMax;
    int VMin;
    int VMax;
    int ShapeItem;
    int mSize;
    int Filter;
    int FilterSize;
    int Area;
public:
    HSVImageHandle(int Filter=0,int FilterSize=0,int HMin=0,int HMax=180,int SMin=0,int SMax=255,int VMin=0,int VMax=255,int ShapeItem=0,int mSize=1);
    ~HSVImageHandle();
    Mat Handle(Mat image) override;
    static HSVImageHandle *mInstance;
    static HSVImageHandle* GetInstance();
    void SetParameter(int Filter,int FilterSize,int HMin,int HMax,int SMin,int SMax,int VMin,int VMax,int ShapeItem,int mSize,int Area);
signals:
    void SendCoordinate(QList<QPoint> point,QList<float> angle);
public slots:
    void SetArea(int area);
};

#endif // HSVIMAGEHANDLE_H
