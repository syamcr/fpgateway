#ifndef BLEDEVICEMANAGER_H
#define BLEDEVICEMANAGER_H

#include <QObject>
#include <list>
#include <QMutex>
#include <QBluetoothDeviceInfo>
#include <QBluetoothDeviceDiscoveryAgent>
#include <QLowEnergyController>

#include "bledevice.h"



class BleDeviceManager : public QObject
{
    Q_OBJECT

private:
    QBluetoothDeviceDiscoveryAgent *m_DiscoveryAgent;
    std::list<BleDevice*>           m_Devices;
    QMutex                          m_Mutex;

public:
    explicit BleDeviceManager(QObject *parent = nullptr);
    void shutdown();

private slots:
    void deviceDiscovered(const QBluetoothDeviceInfo &device);
    void deviceIdentificationChanged(BleDevice *device);
    void scanCompleted();

signals:
    void message(QString msg);
    void deviceConnected(BleDevice *device);

};

#endif // BLEDEVICEMANAGER_H
