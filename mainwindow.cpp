#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QString>
#include <QFileDialog>
#include <QPixmap>
#include <QMessageBox>
#include <opencv2/tracking.hpp>
#include <opencv2/core/ocl.hpp>
#include <opencv2/photo.hpp>
#include <opencv2/objdetect.hpp>
#include <opencv2/xfeatures2d.hpp>
#include <opencv2/features2d.hpp>
#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/stitching.hpp>
#include <opencv2/video.hpp>
#include <opencv2/bgsegm.hpp>
#include "opencv2/imgproc/imgproc.hpp"
#include <QRect>
#include <QXmlStreamWriter>
#include <QThread>
#include <QDebug>
#include <QTimer>
#include "ImageHandle/blobimagehandle.h"
#include "ImageHandle/hsvimagehandle.h"
#include "ImageHandle/templatematchimagehandle.h"
#include <QTableWidgetItem>
#include <QMetaMethod>
#include <QMutexLocker>
#include "Camera/haikangcamera.h"
#include "./common/Global.h"
#include "opencv2/features2d.hpp"
#include "opencv2/xfeatures2d.hpp"
#include "./TemplateMatch/Interface.h"
#include <QProgressDialog>
using namespace cv::xfeatures2d;
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);    
    qRegisterMetaType<Mat>("Mat");
    qRegisterMetaType<QList<QString>>("QList<QString>");
    qRegisterMetaType<QList<QPoint>>("QList<QPoint>");
    qRegisterMetaType<QList<float>>("QList<float>");
    ui->DetailFrame->setHidden(Detail);
    //绑定设备列表
    mDeviceEnumThread.setAutoDelete(false);
    mOpenCameraThread.setAutoDelete(false);
    mCloseCameraThread.setAutoDelete(false);
    mImageHandleThread.setAutoDelete(false);
    connect(ui->OpenImage,SIGNAL(clicked(bool)),this,SLOT(OpenImage_clicked(bool)));
    connect(ui->HSV,SIGNAL(clicked(bool)),this,SLOT(HSV_clicked(bool)));
    connect(ui->Blob,SIGNAL(clicked(bool)),this,SLOT(Blob_clicked(bool)));
    connect(ui->TemplateMatch,SIGNAL(clicked(bool)),this,SLOT(TemplateMatch_clicked(bool)));
    connect(ui->Filter,SIGNAL(activated(int)),this,SLOT(Filter_clicked(int)));
    connect(ui->MinHorizontalSlider,SIGNAL(valueChanged(int)),this,SLOT(ThresholdMinValue(int)));
    connect(ui->MaxHorizontalSlider,SIGNAL(valueChanged(int)),this,SLOT(ThresholdMaxValue(int)));
    connect(ui->HMaxSlider,SIGNAL(valueChanged(int)),this,SLOT(HMaxSlider(int)));
    connect(ui->SMaxSlider,SIGNAL(valueChanged(int)),this,SLOT(SMaxSlider(int)));
    connect(ui->VMaxSlider,SIGNAL(valueChanged(int)),this,SLOT(VMaxSlider(int)));
    connect(ui->HMinSlider,SIGNAL(valueChanged(int)),this,SLOT(HMinSlider(int)));
    connect(ui->SMinSlider,SIGNAL(valueChanged(int)),this,SLOT(SMinSlider(int)));
    connect(ui->VMinSlider,SIGNAL(valueChanged(int)),this,SLOT(VMinSlider(int)));
    connect(ui->ShapeHandle,SIGNAL(activated(int)),this,SLOT(ShapeFilter(int)));
    connect(ui->OPenCamera,SIGNAL(clicked(bool)),this,SLOT(OpenCamera(bool)));
    connect(ui->HSVShapeHandle,SIGNAL(activated(int)),this,SLOT(HSVShapeFilter(int)));    
    //枚举设备定时器
    connect(&mDeviceTimer,SIGNAL(timeout()),this,SLOT(DeviceEnum()));
    //枚举设备线程
    connect(&mDeviceEnumThread,SIGNAL(SendDeviceInfo(QList<QString>)),this,SLOT(ReceiveDeviceInfo(QList<QString>)));
    connect(ui->Display,SIGNAL(SendHSV(int,int,int)),this,SLOT(SetHSV(int,int,int)));
    //监听TabWidget
    connect(ui->tabWidget,SIGNAL(currentChanged(int)),ui->Display,SLOT(TabWidgetChange(int)));
    mDeviceTimer.start(2000);
    ui->Display->setAlignment(Qt::AlignCenter);
    //初始化相机
    HaiKangCamera* camera = new HaiKangCamera;
    //获取图像类型
    connect(ui->tabWidget,SIGNAL(currentChanged(int)),camera,SLOT(SetGrayImage(int)));
    //绑定图像处理函数
    connect(this,SIGNAL(SendImageHandle(ImageHandle *)),&mImageHandleThread,SLOT(SetImageHandle(ImageHandle*)));
    //获取图片线程
    connect(&mImageHandleThread,SIGNAL(SendImage(Mat)),this,SLOT(ReceiveImage(Mat)));
    //获取相机打开情况
    connect(&mOpenCameraThread,SIGNAL(SendOpenCameraState(bool)),this,SLOT(OpenCameraState(bool)));
    mCamera = camera;
    //开启处理图像线程
    QThreadPool::globalInstance()->start(&mImageHandleThread);
    connect(ui->Display,SIGNAL(SendRegion(Mat)),this,SLOT(ReceiveImage(Mat)));
    connect(this,SIGNAL(ClearClipImageList()),ui->Display,SLOT(ClearClipImageList()));
    connect(ui->DetailButton,SIGNAL(clicked(bool)),this,SLOT(BLOBDetail(bool)));
}

MainWindow::~MainWindow()
{
    delete ui;
    if(mCamera != NULL)
    {
        delete mCamera;
        mCamera = NULL;
    }
    mImageHandleThread.Stop();
}


void MainWindow::OpenImage_clicked(bool check)
{
    Q_UNUSED(check)
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"),"C://",tr("Images (*.png *.bmp *.jpg)"));
    if (fileName.isEmpty())
    {
        //QMessageBox::warning(NULL, "提示", "未选择数据文件", "确定");
        return;
    }
    QImage image;
    image.load(fileName);
    (Utils::QImage_to_cvMat(image)).copyTo(mImage);
    ui->Display->setPixmap(QPixmap::fromImage(image).scaled(ui->Display->size(),Qt::KeepAspectRatio,Qt::SmoothTransformation));
    ClearClipImageList();
}
void MainWindow::Filter_clicked(int item)
{
    FilterImage.create(mImage.rows,mImage.cols,mImage.type());
    if(mImage.empty())
    {
        //QMessageBox::warning(NULL, "提示", "未加载图片", "确定");
        return;
    }
    if(item == 0)
    {
        ui->Display->setPixmap(QPixmap::fromImage(Utils::cvMat_to_QImage(mImage)).scaled(ui->Display->size(),Qt::KeepAspectRatio,Qt::SmoothTransformation));
        return ;
    }
    else if(item == 1)
    {
        //中值滤波
        cv::medianBlur(mImage,FilterImage,ui->TemplateSize->value());
        qDebug()<<"中值滤波"<<endl;
    }
    else if(item == 2)
    {
        //均值滤波
        blur(mImage,FilterImage,Size(ui->TemplateSize->value(),ui->TemplateSize->value()));
        qDebug()<<"均值滤波"<<endl;
    }
    else
    {
        //高斯滤波
        GaussianBlur(mImage,FilterImage,Size(ui->TemplateSize->value(),ui->TemplateSize->value()),0,0);
        qDebug()<<"高斯滤波"<<endl;
    }
    ui->Display->clear();
    ui->Display->setPixmap(QPixmap::fromImage(Utils::cvMat_to_QImage(FilterImage)).scaled(ui->Display->size(),Qt::KeepAspectRatio,Qt::SmoothTransformation));
}
void MainWindow::HSV_clicked(bool check)
{
    //写HSV配置文件
    QFile file("Config.xml");
    if(file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate))
    {
        //标志颜色提取
        QXmlStreamWriter stream(&file);
        stream.setAutoFormatting(true);
        stream.writeStartDocument();
        stream.writeStartElement("HSV");
        stream.writeAttribute("HMin", QString::number(ui->HMinSlider->value()));
        stream.writeAttribute("HMax", QString::number(ui->HMaxSlider->value()));
        stream.writeAttribute("SMin", QString::number(ui->SMinSlider->value()));
        stream.writeAttribute("SMax", QString::number(ui->SMaxSlider->value()));
        stream.writeAttribute("VMin", QString::number(ui->VMinSlider->value()));
        stream.writeAttribute("VMax", QString::number(ui->VMaxSlider->value()));
        stream.writeEndDocument();
        file.close();
    }
    //获取HSV句柄
    HSVImageHandle *handle = HSVImageHandle::GetInstance();
    connect(ui->HSVSpinBoxArea,SIGNAL(valueChanged(int)),handle,SLOT(SetArea(int)));
    //static const QMetaMethod valueChangedSignal = QMetaMethod::fromSignal(&HSVImageHandle::SendCoordinate);
    //如果没有连接过，则连接
    //if (!handle->isSignalConnected(valueChangedSignal)) {
        connect(handle,SIGNAL(SendCoordinate(QList<QPoint>,QList<float>)),this,SLOT(ReceiveHSVInfo(QList<QPoint>,QList<float>)));
    //}
    emit SendImageHandle(handle);
    //设置HSV参数int HMin,int HMax,int SMin,int SMax,int VMin,int VMax,int MinThreshold,int MaxThreshold
    HSVImageHandle::GetInstance()->SetParameter(ui->Filter->currentIndex(),ui->TemplateSize->value(),\
                                               ui->HMinSlider->value(),ui->HMaxSlider->value(),\
                                               ui->SMinSlider->value(),ui->SMaxSlider->value(),\
                                               ui->VMinSlider->value(),ui->VMaxSlider->value(),\
                                               ui->HSVShapeHandle->currentIndex(),ui->HSVSizeDilation->value(),\
                                               ui->HSVSpinBoxArea->value());
    //判断相机是否打开
    //if(!mIsOpen)
    {
        OpenCamera(true);
    }
}
void MainWindow::Blob_clicked(bool check)
{
    //写BLOB配置文件
    QFile file("Config.xml");
    if(file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate))
    {
        //标志颜色提取
        QXmlStreamWriter stream(&file);
        stream.setAutoFormatting(true);
        stream.writeStartDocument();
        stream.writeStartElement("BLOB");
        stream.writeAttribute("ThresholdStep",QString::number(ui->spinBox_ThresholdStep->value()));
        stream.writeAttribute("MinThreshold", QString::number(ui->spinBox_MinThreshold->value()));
        stream.writeAttribute("MaxThreshold", QString::number(ui->spinBox_MaxThreshold->value()));
        stream.writeAttribute("Repeat", QString::number(ui->spinBox_Repeat->value()));
        stream.writeAttribute("MinDistance", QString::number(ui->spinBox_MinDistance->value()));
        stream.writeAttribute("ColorDistrict", ui->checkBox_CircleDistrict->isChecked()?"true":"false");
        stream.writeAttribute("ColorDistrictCount", QString::number(ui->spinBox_ColorDistrictValue->value()));

        stream.writeAttribute("DistrictArea", ui->checkBox_DistrictArea->isChecked()?"true":"false");
        stream.writeAttribute("MinArea", QString::number(ui->spinBox_MinArea->value()));
        stream.writeAttribute("MaxArea", QString::number(ui->spinBox_MaxArea->value()));

        stream.writeAttribute("DistrictCircle", ui->checkBox_CircleDistrict->isChecked()?"true":"false");
        stream.writeAttribute("MinCircle", QString::number(ui->spinBox_MinCircle->value()));
        stream.writeAttribute("MaxCircle", QString::number(ui->spinBox_MaxCircle->value()));

        stream.writeAttribute("DistrictInertia", ui->checkBox_Inertia->isChecked()?"true":"false");
        stream.writeAttribute("MinInertia", QString::number(ui->spinBox_MinInertia->value()));
        stream.writeAttribute("MaxInertia", QString::number(ui->spinBox_MaxInertia->value()));

        stream.writeAttribute("DistrictConvexity", ui->checkBox_Convexity->isChecked()?"true":"false");
        stream.writeAttribute("MinConvexity", QString::number(ui->spinBox_MinConvexity->value()));
        stream.writeAttribute("MaxConvexity", QString::number(ui->spinBox_MaxConvexity->value()));

        stream.writeEndDocument();
        file.close();
    }
    //获取BLOB句柄
    BLOBImageHandle *BLOB = BLOBImageHandle::GetInstance();
    SimpleBlobDetector::Params pDefaultBLOB;
    // 初始化BLOB参数
    pDefaultBLOB.thresholdStep = ui->spinBox_ThresholdStep->value();
    pDefaultBLOB.minThreshold = ui->spinBox_MinThreshold->value();
    pDefaultBLOB.maxThreshold = ui->spinBox_MaxThreshold->value();
    pDefaultBLOB.minRepeatability = ui->spinBox_Repeat->value();
    pDefaultBLOB.minDistBetweenBlobs = ui->spinBox_MinDistance->value();
    pDefaultBLOB.filterByColor = ui->checkBox_ColorDistrict->isChecked();
    pDefaultBLOB.blobColor = ui->spinBox_ColorDistrictValue->value();
    pDefaultBLOB.filterByArea = ui->checkBox_DistrictArea->isChecked();;
    pDefaultBLOB.minArea = ui->spinBox_MinArea->value();
    pDefaultBLOB.maxArea = ui->spinBox_MaxArea->value();
    pDefaultBLOB.filterByCircularity = ui->checkBox_CircleDistrict->isChecked();
    pDefaultBLOB.minCircularity = ui->spinBox_MinCircle->value();
    pDefaultBLOB.maxCircularity = ui->spinBox_MaxCircle->value();
    pDefaultBLOB.filterByInertia = ui->checkBox_Inertia->isChecked();
    pDefaultBLOB.minInertiaRatio = ui->spinBox_MinInertia->value();
    pDefaultBLOB.maxInertiaRatio = ui->spinBox_MaxInertia->value();
    pDefaultBLOB.filterByConvexity = ui->checkBox_Convexity->isChecked();
    pDefaultBLOB.minConvexity = ui->spinBox_MinConvexity->value();
    pDefaultBLOB.maxConvexity = ui->spinBox_MaxConvexity->value();
    BLOB->SetParameter(ui->Filter->currentIndex(),ui->TemplateSize->value(),pDefaultBLOB);
    connect(BLOB,SIGNAL(SendCoordinate(QList<QPoint>,QList<float>)),this,SLOT(ReceiveBLOBInfo(QList<QPoint>,QList<float>)));
    emit SendImageHandle(BLOB);
    //判断相机是否打开
    //if(!mIsOpen)
    {
        OpenCamera(true);
    }
}
void MainWindow::TemplateMatch_clicked(bool check)
{
    if(ui->Display->pixmap() == NULL)
    {
        return ;
    }
    //写Template配置文件
    QFile file("Config.xml");
    if(file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate))
    {
        //标志颜色提取
        QXmlStreamWriter stream(&file);
        stream.setAutoFormatting(true);
        stream.writeStartDocument();
        stream.writeStartElement("Template");
        stream.writeAttribute("MinThreshold", QString::number(ui->MinHorizontalSlider->value()));
        stream.writeAttribute("MaxThreshold", QString::number(ui->MaxHorizontalSlider->value()));
        stream.writeAttribute("ShapeFilter",  QString::number(ui->ShapeHandle->currentIndex()));
        stream.writeAttribute("TemplateSize", QString::number(ui->TemplateSize->value()));
        stream.writeEndDocument();
        file.close();
    }
    //获取Template句柄
    TemplateMatchImageHandle *handle = TemplateMatchImageHandle::GetInstance();
    if(ui->TemplateMode->currentIndex() == 0)
    {
        vector<vector<Point>> contours2;
        vector<Vec4i> hierarcy2;
        Mat image = Utils::QImage_to_cvMat(ui->Display->pixmap()->toImage(),true);
        cvtColor(image,image,CV_BGR2GRAY);
        findContours(image, contours2, hierarcy2, CV_RETR_CCOMP, CV_CHAIN_APPROX_NONE);
        if(contours2.size() <= 0)
        {
            return ;
        }
        //获取模板角度
        RotatedRect rect = minAreaRect(contours2[0]);
        Mat img = Mat::zeros(image.rows,image.cols,image.type());
        handle->SetParameter(ui->Filter->currentIndex(),ui->TemplateSize->value(),ui->MinHorizontalSlider->value(),ui->MaxHorizontalSlider->value(),contours2,ui->ShapeHandle->currentIndex(),ui->TemplateSizeDilation->value(),ui->result->value(),rect.angle,0);
    }
    else if(ui->TemplateMode->currentIndex() == 1)
    {
        QProgressDialog progress(tr("正在生成模板..."), tr("取消"), 0, 360, this,Qt::FramelessWindowHint);
        progress.setWindowModality(Qt::WindowModal);
        progress.show();
        Mat image = Utils::QImage_to_cvMat(ui->Display->pixmap()->toImage(),true);
        if(!angle_test(image,&progress,"train",true))
        {
            return ;
        }
        //创建模板
        vector<vector<Point>> contours2;
        handle->SetParameter(ui->Filter->currentIndex(),ui->TemplateSize->value(),0,0,contours2,ui->ShapeHandle->currentIndex(),ui->TemplateSizeDilation->value(),ui->result->value(),0,1);
    }
    connect(handle,SIGNAL(SendCoordinate(QList<QPoint>,QList<float>)),this,SLOT(ReceiveTempLateInfo(QList<QPoint>,QList<float>)));
    emit SendImageHandle(handle);
    OpenCamera(true);
}
void MainWindow::ThresholdMinValue(int min)
{
    ImagetoThreshold();
    ui->MinValue->setText(QString::number(min));    
}
void MainWindow::ThresholdMaxValue(int max)
{
    ImagetoThreshold();
    ui->MaxValue->setText(QString::number(max));
}
void MainWindow::HMaxSlider(int H)
{
    ImagetoHSV();
    ui->HMaxValue->setText(QString::number(H));
}
void MainWindow::SMaxSlider(int S)
{
    ImagetoHSV();
    ui->SMaxValue->setText(QString::number(S));
}
void MainWindow::VMaxSlider(int V)
{
    ImagetoHSV();
    ui->VMaxValue->setText(QString::number(V));
}
void MainWindow::HMinSlider(int H)
{
    ImagetoHSV();
    ui->HMinValue->setText(QString::number(H));
}
void MainWindow::SMinSlider(int S)
{
    ImagetoHSV();
    ui->SMinValue->setText(QString::number(S));
}
void MainWindow::VMinSlider(int V)
{
    ImagetoHSV();
    ui->VMinValue->setText(QString::number(V));
}

void MainWindow::ImagetoHSV()
{
    if(mImage.empty())
    {
        //QMessageBox::warning(NULL, "提示", "未加载图片", "确定");
        return;
    }
    else if(mImage.channels() == 1)
    {
        //QMessageBox::warning(NULL, "提示", "图片非三通单", "确定");
        return;
    }
    //判断图像是否滤波
    if(ui->Filter->currentIndex() == 0)
    {
        //无滤波
        cvtColor(mImage, HSVImage, COLOR_BGR2HSV);
    }
    else
    {
        if(FilterImage.empty())
        {
            //无滤波
            cvtColor(mImage, HSVImage, COLOR_BGR2HSV);
        }
        else
        {
            //无滤波
            cvtColor(FilterImage, HSVImage, COLOR_BGR2HSV);
        }
    }
    IplImage tmp=IplImage(HSVImage);//添加的代码
    IplImage *mask;
    CvArr* arr = (CvArr*)&tmp;//添加的代码
    mask=cvCreateImage(cvGetSize(arr),8,1);
    cvInRangeS(arr,Scalar(qMin(ui->HMinSlider->value(),ui->HMaxSlider->value()),qMin(ui->SMinSlider->value(),ui->SMaxSlider->value()),qMin(ui->VMinSlider->value(),ui->VMaxSlider->value()),0),\
               Scalar(qMax(ui->HMinSlider->value(),ui->HMaxSlider->value()),qMax(ui->SMinSlider->value(),ui->SMaxSlider->value()),qMax(ui->VMinSlider->value(),ui->VMaxSlider->value()),0),mask);
    HSVImage = cv::cvarrToMat(mask);
    if(HSVImage.empty())
    {
        qDebug()<<"empty!!"<<endl;
    }
    ui->Display->clear();
    ui->Display->setPixmap(QPixmap::fromImage(Utils::cvMat_to_QImage(HSVImage)).scaled(ui->Display->size(),Qt::KeepAspectRatio,Qt::SmoothTransformation));
}
void MainWindow::ImagetoThreshold()
{
    //判断图片是否存在
    if(mImage.empty())
    {
        //QMessageBox::warning(NULL, "提示", "未加载图片", "确定");
        return;
    }
    if(ui->Filter->currentIndex() == 0)
    {
        //先把图片转成灰度
        if(mImage.type() != CV_8UC1)
        {
            cvtColor(mImage,ThresholdImage,CV_BGR2GRAY);
            threshold(ThresholdImage,ThresholdImage,ui->MinHorizontalSlider->value(),ui->MaxHorizontalSlider->value(),THRESH_BINARY);
        }
        else
        {
            threshold(mImage,ThresholdImage,ui->MinHorizontalSlider->value(),ui->MaxHorizontalSlider->value(),THRESH_BINARY);
        }
    }
    else
    {
        //先把图片转成灰度
        if(FilterImage.type() != CV_8UC1)
        {
            cvtColor(FilterImage,ThresholdImage,CV_BGR2GRAY);
            threshold(ThresholdImage,ThresholdImage,ui->MinHorizontalSlider->value(),ui->MaxHorizontalSlider->value(),THRESH_BINARY);
        }
        else
        {
            threshold(mImage,ThresholdImage,ui->MinHorizontalSlider->value(),ui->MaxHorizontalSlider->value(),THRESH_BINARY);
        }
    }
    qDebug()<<ui->MinHorizontalSlider->value()<<":"<<ui->MaxHorizontalSlider->value()<<endl;

    //imshow("threshold",ThresholdImage);
    ui->Display->clear();
    ui->Display->setPixmap(QPixmap::fromImage(Utils::cvMat_to_QImage(ThresholdImage)).scaled(ui->Display->size(),Qt::KeepAspectRatio,Qt::SmoothTransformation));
}
//形态学
void MainWindow::ShapeFilter(int item)
{
    if(item != 0 && !ThresholdImage.empty())
    {
        Mat kernel = getStructuringElement(MORPH_RECT,Size(ui->TemplateSizeDilation->value(),ui->TemplateSizeDilation->value()));//创建结构元素大小为3*3
        morphologyEx(ThresholdImage,ShapeFilterImage,item-1,kernel);
        ui->Display->setPixmap(QPixmap::fromImage(Utils::cvMat_to_QImage(ShapeFilterImage)).scaled(ui->Display->size(),Qt::KeepAspectRatio,Qt::SmoothTransformation));
        kernel.release();
    }
    else if(item == 0)
    {
        ui->Display->setPixmap(QPixmap::fromImage(Utils::cvMat_to_QImage(ThresholdImage)).scaled(ui->Display->size(),Qt::KeepAspectRatio,Qt::SmoothTransformation));
    }

}
//打开或关闭相机
void MainWindow::OpenCamera(bool isOpen)
{
    //如果触发是这个清除ImageHandle
    if(sender()==ui->OPenCamera)
    {
        emit SendImageHandle(NULL);
    }
    if(ui->OPenCamera->text().contains(tr("打开相机")))
    {
        if(ui->comboBox_Device->count() == 0)
        {
            //没有设备可以打开直接退出
            return ;
        }
        mOpenCameraThread.SetIndex(ui->comboBox_Device->currentIndex());
        //打开相机
        QThreadPool::globalInstance()->start(&mOpenCameraThread);
        ClearClipImageList();
    }
    else
    {
        //关闭相机
        QThreadPool::globalInstance()->start(&mCloseCameraThread);
        ui->OPenCamera->setText(tr("打开相机"));
        ClearClipImageList();
    }
}

void MainWindow::ReceiveImage(Mat Image)
{
    qDebug()<<sender()<<endl;
    if(Image.empty())
    {
        return ;
    }
    mImage = Image;
    ui->Display->setPixmap(QPixmap::fromImage(Utils::cvMat_to_QImage(mImage)).scaled(ui->Display->size(),Qt::KeepAspectRatio,Qt::SmoothTransformation));
    mDeviceTimer.stop();
}

void MainWindow::DeviceEnum()
{
    //固定窗口大小
    if(isFix)
    {
        setFixedSize(width(),height());
        isFix = !isFix;
    }
    QThreadPool::globalInstance()->start(&mDeviceEnumThread);
}

void MainWindow::ReceiveDeviceInfo(QList<QString> list)
{
    QString str;
    if(list.size() == 0)
    {
        ui->comboBox_Device->clear();
        ui->comboBox_Device->addItem("没有找到设备!");
        return ;
    }
    foreach (str, list) {
        //qDebug()<<str<<endl;
        if(str == tr("没有找到设备!"))
        {
            if(!(ui->comboBox_Device->findText(tr("没有找到设备!")) != -1 && ui->comboBox_Device->count() == 1))
            {
                ui->comboBox_Device->clear();
                ui->comboBox_Device->addItem(str);
            }
            //使能打开相机按钮
            if(ui->OPenCamera->isEnabled())
            {
                ui->OPenCamera->setEnabled(false);
            }
            break;
        }
        else if(ui->comboBox_Device->findText(str) == -1)
        {
            int item;
            if((item = ui->comboBox_Device->findText(tr("没有找到设备!"))) != -1)
            {
                ui->comboBox_Device->removeItem(item);
            }
            ui->comboBox_Device->addItem(str);
            if(!ui->OPenCamera->isEnabled())
            {
                ui->OPenCamera->setEnabled(true);
            }
        }
    }
}

void MainWindow::HSVShapeFilter(int item)
{
    if(item != 0 && !HSVImage.empty())
    {
        Mat kernel = getStructuringElement(MORPH_RECT,Size(ui->HSVSizeDilation->value(),ui->HSVSizeDilation->value()));//创建结构元素大小为3*3
        morphologyEx(HSVImage,HSVShapeImage,item-1,kernel);
        ui->Display->setPixmap(QPixmap::fromImage(Utils::cvMat_to_QImage(HSVShapeImage)).scaled(ui->Display->size(),Qt::KeepAspectRatio,Qt::SmoothTransformation));
        kernel.release();
    }
    else if(item == 0)
    {
        ui->Display->setPixmap(QPixmap::fromImage(Utils::cvMat_to_QImage(HSVImage)).scaled(ui->Display->size(),Qt::KeepAspectRatio,Qt::SmoothTransformation));
    }
}
void MainWindow::SetHSV(int H,int S,int V)
{
    ui->HMaxSlider->setValue(180);
    ui->HMinSlider->setValue(H);
    ui->SMaxSlider->setValue(255);
    ui->SMinSlider->setValue(S);
    ui->VMaxSlider->setValue(255);
    ui->VMinSlider->setValue(V);
}
void MainWindow::ReceiveHSVInfo(QList<QPoint> Point,QList<float> angle)
{
    ui->HSVTableWidget->clearContents();
    if(Point.count() != angle.count() && angle.count() > 0)
    {
        return ;
    }
    while(ui->HSVTableWidget->rowCount()>Point.count())
    {
        ui->HSVTableWidget->removeRow(0);
    }
    int i = 0;
    foreach (QPoint point, Point) {
        if(i>=ui->HSVTableWidget->rowCount())
        {
            ui->HSVTableWidget->insertRow(ui->HSVTableWidget->rowCount());
        }
        QTableWidgetItem *PointItem = new QTableWidgetItem;
        PointItem->setTextAlignment(Qt::AlignCenter);
        PointItem->setText(tr("X:")+QString::number(point.x())+tr("  Y:")+QString::number(point.y()));
        ui->HSVTableWidget->setItem(i,0,PointItem);
        QTableWidgetItem *AngleItem = new QTableWidgetItem;
        AngleItem->setTextAlignment(Qt::AlignCenter);
        AngleItem->setText(QString::number(angle.at(i)));
        ui->HSVTableWidget->setItem(i,1,AngleItem);
        i++;
    }
}

void MainWindow::OpenCameraState(bool state)
{
    if(state)
    {
        ui->OPenCamera->setText(tr("关闭相机"));
    }
    else
    {
        ui->OPenCamera->setText(tr("打开相机"));
    }
}

void MainWindow::ReceiveBLOBInfo(QList<QPoint> Point,QList<float> angle)
{
    ui->BLOBTableWidget->clearContents();
    if(Point.count() != angle.count() && angle.count() > 0)
    {
        return ;
    }
    while(ui->BLOBTableWidget->rowCount()>Point.count())
    {
        ui->BLOBTableWidget->removeRow(0);
    }
    int i = 0;
    foreach (QPoint point, Point) {
        if(i>=ui->BLOBTableWidget->rowCount())
        {
            ui->BLOBTableWidget->insertRow(ui->BLOBTableWidget->rowCount());
        }
        QTableWidgetItem *PointItem = new QTableWidgetItem;
        PointItem->setTextAlignment(Qt::AlignCenter);
        PointItem->setText(tr("X:")+QString::number(point.x())+tr("  Y:")+QString::number(point.y()));
        ui->BLOBTableWidget->setItem(i,0,PointItem);
        QTableWidgetItem *AngleItem = new QTableWidgetItem;
        AngleItem->setTextAlignment(Qt::AlignCenter);
        AngleItem->setText(QString::number(angle.at(i)));
        ui->BLOBTableWidget->setItem(i,1,AngleItem);
        i++;
    }
}

void MainWindow::ReceiveTempLateInfo(QList<QPoint> Point,QList<float> angle)
{
    ui->TemplateTableWidget->clearContents();
    if(Point.count() != angle.count() && angle.count() > 0)
    {
        return ;
    }
    while(ui->TemplateTableWidget->rowCount()>Point.count())
    {
        ui->TemplateTableWidget->removeRow(0);
    }
    int i = 0;
    foreach (QPoint point, Point) {
        if(i>=ui->TemplateTableWidget->rowCount())
        {
            ui->TemplateTableWidget->insertRow(ui->TemplateTableWidget->rowCount());
        }
        QTableWidgetItem *PointItem = new QTableWidgetItem;
        PointItem->setTextAlignment(Qt::AlignCenter);
        PointItem->setText(tr("X:")+QString::number(point.x())+tr("  Y:")+QString::number(point.y()));
        ui->TemplateTableWidget->setItem(i,0,PointItem);
        QTableWidgetItem *AngleItem = new QTableWidgetItem;
        AngleItem->setTextAlignment(Qt::AlignCenter);
        AngleItem->setText(QString::number(angle.at(i)));
        ui->TemplateTableWidget->setItem(i,1,AngleItem);
        i++;
    }
}

void MainWindow::BLOBDetail(bool)
{
    Detail = !Detail;
    ui->DetailFrame->setHidden(Detail);
}
