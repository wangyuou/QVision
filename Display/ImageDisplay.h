#ifndef DISPLAY_H
#define DISPLAY_H
#include <QLabel>
#include <QMouseEvent>
#include <opencv2/opencv.hpp>
#include <QPoint>
#include <QList>
using namespace cv;
class ImageDisplay : public QLabel
{
    Q_OBJECT
signals:
    void SendHSV(int H,int S,int V);
    void SendThresholdImage(Mat Image);
    void SendRegion(Mat img);
public:
    ImageDisplay(QWidget* parent = 0);    
protected:
    void mouseReleaseEvent(QMouseEvent *ev)override;
    void mousePressEvent(QMouseEvent *ev)override;
    void mouseMoveEvent(QMouseEvent *ev)override;
private slots:
    void TabWidgetChange(int state);
    void ClearClipImageList();
private:
    Mat TempImage;
    bool isPress=false;
    QPoint StartPoint;
    QPoint EndPoint;
    int CurrentState=0;
    QList<Mat> SaveClipImageList;
    bool isMove = false;
    bool isOnce = true;
};

#endif // DISPLAY_H
