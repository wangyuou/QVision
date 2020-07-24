#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "Service/capturethread.h"
#include "Common/utils.h"
#include <QTimer>
#include "Service/deviceenumthread.h"
#include <QThreadPool>
#include <QStandardItemModel>
#include <QList>
#include <QPoint>
#include <QMutex>
#include "Service/opencamerathread.h"
#include "Service/closecamerathread.h"
#include "ImageHandle/imagehandle.h"
#include "Service/imagehandlethread.h"
QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{    
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void ImagetoHSV();
    void ImagetoThreshold();
private slots:
    void OpenImage_clicked(bool check);
    void Filter_clicked(int item);
    void HSV_clicked(bool check);
    void Blob_clicked(bool check);
    void TemplateMatch_clicked(bool check);
    void ThresholdMaxValue(int max);
    void ThresholdMinValue(int min);
    void HMaxSlider(int H);
    void SMaxSlider(int S);
    void VMaxSlider(int V);
    void HMinSlider(int H);
    void SMinSlider(int S);
    void VMinSlider(int V);
    void ShapeFilter(int item);
    void OpenCamera(bool isOpen);
    void ReceiveImage(Mat image);
    void DeviceEnum();
    void ReceiveDeviceInfo(QList<QString>list);
    void HSVShapeFilter(int item);
    void SetHSV(int H,int S,int V);
    void ReceiveHSVInfo(QList<QPoint> Point,QList<float> angle);
    void OpenCameraState(bool);
    void ReceiveBLOBInfo(QList<QPoint> Point,QList<float> angle);
    void ReceiveTempLateInfo(QList<QPoint> Point,QList<float> angle);
    void BLOBDetail(bool);
private:
    Ui::MainWindow *ui;
    Mat mImage;
    Mat ThresholdImage;
    Mat HSVImage;
    Mat FilterImage;
    Mat ShapeFilterImage;
    QTimer mDeviceTimer;
    DeviceEnumThread mDeviceEnumThread;
    OpenCameraThread mOpenCameraThread;
    CloseCameraThread mCloseCameraThread;
    ImageHandleThread mImageHandleThread;
    Mat HSVShapeImage;
    bool isFix=true;
    QMutex DeviceListLock;
    bool Detail=true;

signals:
    void SendImageHandle(ImageHandle *handle);
    void ClearClipImageList();
};
#endif // MAINWINDOW_H
