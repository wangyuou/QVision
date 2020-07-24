QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11
# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

QMAKE_CXXFLAGS += -std=c++11
QMAKE_CXXFLAGS += -fopenmp -O2 -mavx2
LIBS += -lgomp -lpthread

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    Camera/haikangcamera.cpp \
    ImageHandle/blobimagehandle.cpp \
    ImageHandle/hsvimagehandle.cpp \
    ImageHandle/imagehandle.cpp \
    ImageHandle/templatematchimagehandle.cpp \
    Service/closecamerathread.cpp \
    Service/deviceenumthread.cpp \
    Service/imagehandlethread.cpp \
    Service/opencamerathread.cpp \
    Common/Global.cpp \
    Common/utils.cpp \
    Display/ImageDisplay.cpp \
    TemplateMatch/Interface.cpp \
    TemplateMatch/line2Dup.cpp \
    main.cpp \
    Protocol/Protocol.cpp\
    mainwindow.cpp

HEADERS += \
    Camera/camerabasic.h \
    Camera/haikangcamera.h \
    ImageHandle/blobimagehandle.h \
    ImageHandle/hsvimagehandle.h \
    ImageHandle/imagehandle.h \
    ImageHandle/templatematchimagehandle.h \
    Service/closecamerathread.h \
    Service/deviceenumthread.h \
    Service/imagehandlethread.h \
    Service/opencamerathread.h \
    Common/Global.h \
    Common/concurrentqueue.h \
    Common/utils.h \
    Display/ImageDisplay.h \
    TemplateMatch/Interface.h \
    TemplateMatch/line2Dup.h \
    TemplateMatch/mipp.h \
    TemplateMatch/mipp_impl_AVX.hxx \
    TemplateMatch/mipp_impl_AVX512.hxx \
    TemplateMatch/mipp_impl_NEON.hxx \
    TemplateMatch/mipp_impl_SSE.hxx \
    TemplateMatch/mipp_object.hxx \
    TemplateMatch/mipp_scalar_op.h \
    TemplateMatch/mipp_scalar_op.hxx \
    Protocol/Protocol.h\
    mainwindow.h

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
# opencv
INCLUDEPATH +=  "E:/halcon/opencv/output/install/include/"
LIBS+= -L "E:/halcon/opencv/output/lib/libopencv_*.a"
# camera sdk
INCLUDEPATH +=  "C:/Program Files (x86)/MVS/Development/Includes/"
LIBS+= -L "C:/Program Files (x86)/MVS/Development/Libraries/win64/MvCameraControl.lib"
