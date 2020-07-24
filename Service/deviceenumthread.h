#ifndef DEVICEENUMTHREAD_H
#define DEVICEENUMTHREAD_H
#include <QObject>
#include <QRunnable>
class DeviceEnumThread:public QObject,public QRunnable
{
    Q_OBJECT
signals:
    void SendDeviceInfo(QList<QString>);
public:
    DeviceEnumThread();
    void run()override;
};

#endif // DEVICEENUMTHREAD_H
