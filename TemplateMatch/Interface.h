#ifndef INTERFACE_H
#define INTERFACE_H
#include "line2Dup.h"
#include <memory>
#include <iostream>
#include <assert.h>
#include <chrono>
#include <QProgressDialog>
#include <QDebug>
using namespace std;
using namespace cv;
bool angle_test(Mat img,QProgressDialog *Progress,string mode,bool use_rot);
#endif
