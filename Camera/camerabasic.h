#ifndef CAMEREBASIC_H
#define CAMEREBASIC_H
#include <QObject>
#include <QList>
#include "opencv2/highgui/highgui.hpp"
using namespace cv;
class CameraBasic : public QObject
{
    Q_OBJECT
public:
    CameraBasic(){};
    virtual ~CameraBasic(){};
    virtual bool OpenCamera(int index) = 0;
    virtual bool CloseCamera() = 0;
    virtual QList<QString>EnumDeviceInfo() = 0;
};

#endif // CAMEREBASIC_H
