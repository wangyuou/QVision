#include <QApplication>
#include "mainwindow.h"
#include <QDesktopWidget>
#include <QMenuBar>
#include <QDebug>
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
//    QFile stylesheet("formStyle.qss");
//    stylesheet.open(QFile::ReadWrite);
//    QString setSheet = QLatin1String(stylesheet.readAll());
//    a.setStyleSheet(setSheet);
    MainWindow w;
    w.showMaximized();
    return a.exec();
}
