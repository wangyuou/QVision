#ifndef BLOBIMAGEHANDLE_H
#define BLOBIMAGEHANDLE_H
#include "imagehandle.h"
#include <QPoint>
#include <QList>
class BLOBImageHandle : public ImageHandle
{
    Q_OBJECT
signals:
    void SendCoordinate(QList<QPoint>,QList<float>);
private:
    SimpleBlobDetector::Params pDefaultBLOB;
    int Filter;
    int FilterSize;
public:    
    static BLOBImageHandle *mInstance;
    BLOBImageHandle();
    static BLOBImageHandle* GetInstance();
    Mat Handle(Mat image) override;
    ~BLOBImageHandle();
    void SetParameter(int Filter,int FilterSize,SimpleBlobDetector::Params params);
};
#endif // BLOBIMAGEHANDLE_H
