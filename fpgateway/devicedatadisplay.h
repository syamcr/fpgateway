#ifndef DEVICEDATADISPLAY_H
#define DEVICEDATADISPLAY_H

#include <QWidget>
#include <QLabel>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>

#include "bledevice.h"



class DeviceDataDisplay : public QFrame
{
    Q_OBJECT

private:
    BleDevice   *m_Device;
    QLabel      *m_lblTitle;
    QLabel      *m_lblNameTag;
    QLabel      *m_lblName;
    QLabel      *m_lblDataValue;

    QGridLayout *m_GridLayout;

public:
    DeviceDataDisplay(BleDevice *device, QWidget *parent = nullptr);
    BleDevice *device();

public slots:
    void dataUpdated(BleDevice *device, int data);
    void deviceIdentificationChanged(BleDevice *device);

};

#endif // DEVICEDATADISPLAY_H


