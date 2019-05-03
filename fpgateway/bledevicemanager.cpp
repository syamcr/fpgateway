
#include <QDebug>
#include <QBluetoothLocalDevice>


#include "bledevicemanager.h"
#include "globals.h"
#include "mainwindow.h"



BleDeviceManager::BleDeviceManager(QObject *parent) : QObject(parent)
{
    QBluetoothLocalDevice local_device;
    if(local_device.hostMode() == QBluetoothLocalDevice::HostPoweredOff)
    {
        local_device.powerOn();
        idleAway(5000);
    }

    qDebug() << "Local device: " << local_device.name() << local_device.address() << local_device.hostMode();

    m_DiscoveryAgent = new QBluetoothDeviceDiscoveryAgent(this);
    m_DiscoveryAgent->setLowEnergyDiscoveryTimeout(30000);

    connect(m_DiscoveryAgent, &QBluetoothDeviceDiscoveryAgent::deviceDiscovered,
            this, &BleDeviceManager::deviceDiscovered);
    connect(m_DiscoveryAgent, &QBluetoothDeviceDiscoveryAgent::finished,
            this, &BleDeviceManager::scanCompleted);
    connect(m_DiscoveryAgent, &QBluetoothDeviceDiscoveryAgent::canceled,
            this, &BleDeviceManager::scanCompleted);

    m_DiscoveryAgent->start(QBluetoothDeviceDiscoveryAgent::LowEnergyMethod);

    qDebug() << "Discovery agent started";
}




void BleDeviceManager::shutdown()
{
    m_Mutex.lock();
    for(auto device : m_Devices)
    {
        device->disconnect();
    }
    m_Mutex.unlock();

    m_DiscoveryAgent->stop();
}



void BleDeviceManager::deviceDiscovered(const QBluetoothDeviceInfo &device)
{
    qDebug() << "Found device: " << device.name() << device.address();

    // If device is LowEnergy-device, add it to the list
    if (device.coreConfigurations() & QBluetoothDeviceInfo::LowEnergyCoreConfiguration)
    {
        if(!device.name().startsWith("DataGen")) return;

        m_Mutex.lock();
        bool new_dev = true;
        for(auto dev : m_Devices)
        {
            if(dev->m_DeviceInfo.address() == device.address())
            {
                new_dev = false;
                break;
            }
        }

        if(new_dev)
        {
            BleDevice *ble_dev = new BleDevice(device, this);
            m_Devices.push_back(ble_dev);
            connect(ble_dev, &BleDevice::message, this, &BleDeviceManager::message);
            connect(ble_dev, &BleDevice::deviceIdentificationChanged, this, &BleDeviceManager::deviceIdentificationChanged);
            connect(ble_dev, &BleDevice::deviceConnected,  this, &BleDeviceManager::deviceConnected);
            connect(ble_dev, &BleDevice::dataUpdated, g_MainWindow, &MainWindow::bleDeviceDataUpdated);
        }

        m_Mutex.unlock();

    }
}



void BleDeviceManager::deviceIdentificationChanged(BleDevice *device)
{

    if(device->identifiedDevice() == nullptr) return;

    qDebug() << "Identified " << device->identifiedDevice()->name();

    //check if this is a duplicate
    for(auto dev : m_Devices)
    {
        if(dev == device) continue;

        if( (dev->identifiedDevice() == device->identifiedDevice()) &&
            (dev->bleDeviceInfo().address() != device->bleDeviceInfo().address()) )
        {
            //reject this identification
            device->setIdentifiedDevice(nullptr);
        }
    }

}



void BleDeviceManager::scanCompleted()
{
    qDebug() << "Scan completed";

    idleAway(1000);

    //restart scan
    m_DiscoveryAgent->start(QBluetoothDeviceDiscoveryAgent::LowEnergyMethod);
    //m_DiscoveryAgent->start(QBluetoothDeviceDiscoveryAgent::ClassicMethod);

}




