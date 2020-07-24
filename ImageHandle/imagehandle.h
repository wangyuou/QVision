#ifndef IMAGEHANDLE_H
#define IMAGEHANDLE_H
#include <opencv2/opencv.hpp>
#include <QObject>
using namespace cv;
class ImageHandle : public QObject
{
    Q_OBJECT
public:
    ImageHandle();
    virtual ~ImageHandle();
    virtual Mat Handle(Mat image) = 0;
};

#endif // IMAGEHANDLE_H
