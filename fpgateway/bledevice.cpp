
#include <QDebug>
#include <QtEndian>

#include <vector>

#include "bledevice.h"
#include "globals.h"
#include "bledevicemanager.h"


QTimer* BleDevice::m_ConnectionTimer = nullptr;
int     BleDevice::m_IdentificationScoreThreshold = 2;


BleDevice::BleDevice(const QBluetoothDeviceInfo &dev_info, QObject *parent)
    : QObject(parent),
      m_DeviceInfo(dev_info),
      m_Controller(nullptr),
      m_DataService(nullptr),
      m_IdService(nullptr),
      m_IdentifiedDevice(nullptr)
{

    if(m_ConnectionTimer == nullptr)
    {
        m_ConnectionTimer = new QTimer(g_BleDeviceManager);
        m_ConnectionTimer->start(5000);
    }

    m_Controller = QLowEnergyController::createCentral(m_DeviceInfo, this);

    connect(m_Controller, &QLowEnergyController::discoveryFinished,
            this, &BleDevice::serviceScanFinished);

    connect(m_Controller, static_cast<void (QLowEnergyController::*)(QLowEnergyController::Error)>(&QLowEnergyController::error),
            this, &BleDevice::connectionError);

    connect(m_Controller, &QLowEnergyController::connected,
            this, &BleDevice::connectionSuccessful);

    connect(m_Controller, &QLowEnergyController::disconnected, this, [this]() {
        emit message(m_DeviceInfo.name() + ": LowEnergy controller disconnected");
    });

    connect(m_ConnectionTimer, &QTimer::timeout,
            this, &BleDevice::attemptConnection);


    // Connect
    m_Controller->connectToDevice();

}



BleDevice::~BleDevice()
{
    this->disconnect();
}


void BleDevice::disconnect()
{
    if(m_Controller->state() == QLowEnergyController::ConnectedState)
    {
        m_Controller->disconnectFromDevice();
    }
}

int BleDevice::idScoreThreshold()
{
    return m_IdentificationScoreThreshold;
}

void BleDevice::setIdScoreThreshold(int score)
{
    m_IdentificationScoreThreshold = score;
}



void BleDevice::serviceScanFinished()
{
    emit message(m_DeviceInfo.name() + ": services found = " + QString::number(m_Controller->services().size()));

    for(auto &service : m_Controller->services())
    {
        uint16_t uuid = service.toUInt16();

        if(uuid == 0x180d)
        {
            //this is our heart rate data service
            m_DataService = m_Controller->createServiceObject(service, m_Controller);
        }
        else if(uuid == 0xabcd)
        {
            //this is the ID service
            m_IdService = m_Controller->createServiceObject(service, m_Controller);
        }

    }

    if(m_DataService)
    {
        connect(m_DataService, &QLowEnergyService::stateChanged,
                this, &BleDevice::dataServiceDetailsDiscoveryFinished);

        m_DataService->discoverDetails();
    }

}



void BleDevice::connectionError(QLowEnergyController::Error)
{
    qDebug() << (m_DeviceInfo.name() + ": Cannot connect to remote device.");
}



void BleDevice::connectionSuccessful()
{
    emit message(m_DeviceInfo.name() + ": Controller connected. Searching services...");
    emit deviceConnected(this);

    m_Controller->discoverServices();
}




void BleDevice::attemptConnection()
{
    if(m_Controller->state() == QLowEnergyController::UnconnectedState)
    {
        m_Controller->connectToDevice();
    }
}



void BleDevice::dataServiceDetailsDiscoveryFinished(QLowEnergyService::ServiceState new_state)
{
    if(new_state == QLowEnergyService::ServiceDiscovered)
    {
        m_DataChar = m_DataService->characteristic(QBluetoothUuid(QBluetoothUuid::HeartRateMeasurement));
        if(m_DataChar.isValid())
        {
            QLowEnergyDescriptor notification_desc = m_DataChar.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration);
            if (notification_desc.isValid())
                m_DataService->writeDescriptor(notification_desc, QByteArray::fromHex("0100"));

            connect(m_DataService, &QLowEnergyService::characteristicChanged,
                    this, &BleDevice::dataValueUpdateHandler);
        }
        else
        {
            emit message(m_DeviceInfo.name() + ": HR data service not found");
        }

        if(m_IdService)
        {
            m_IdService->discoverDetails();

            connect(m_IdService, &QLowEnergyService::stateChanged,
                    this, &BleDevice::idServiceDetailsDiscoveryFinished);
        }
    }


}



void BleDevice::idServiceDetailsDiscoveryFinished(QLowEnergyService::ServiceState new_state)
{
    if(new_state == QLowEnergyService::ServiceDiscovered)
    {
        for(auto &chr : m_IdService->characteristics())
        {
            uint16_t uuid = chr.uuid().toUInt16();
            if(uuid == 0xdeed)
            {
                m_IdChar = chr;

                connect(m_IdService, &QLowEnergyService::characteristicRead,
                        this, &BleDevice::fpidValueRead);
                m_IdService->readCharacteristic(m_IdChar);

                break;
            }
        }
    }
}



void BleDevice::fpidValueRead(const QLowEnergyCharacteristic&, const QByteArray &value)
{
    m_FpidValue = value;
    emit message(m_DeviceInfo.name() + ": FPID: " + m_FpidValue.toHex());

    KnownDevice *matched_dev = nullptr;
    int max_score = 0;
    //we have the fingerprint pattern - try to match it to a known device - the one with the highest score
    for(int i = 0; i < g_KnownDevices.size(); ++i)
    {
        KnownDevice *kd = &g_KnownDevices[i];
        int score = kd->getMatchScore(value);
        qDebug() << "Dev: " << kd->name() << "Score: " << score << "  max: " << max_score;
        if(score > max_score)
        {
            max_score = score;
            matched_dev = kd;
        }
    }

    if((max_score >= m_IdentificationScoreThreshold) && (matched_dev != nullptr))
    {
        this->setIdentifiedDevice(matched_dev);
    }
    else
    {
        this->setIdentifiedDevice(nullptr);
    }

}



void BleDevice::dataValueUpdateHandler(const QLowEnergyCharacteristic&, const QByteArray &value)
{
    const uint8_t *data = reinterpret_cast<const uint8_t*>(value.constData());
    uint8_t flags = data[0];

    if (flags & 0x1) // HR 16 bit? otherwise 8 bit
        m_DataValue = qFromLittleEndian<uint16_t>(data + 1);
    else
        m_DataValue = data[1];

    qDebug() << m_DeviceInfo.name() << m_DataValue << (m_IdentifiedDevice ? m_IdentifiedDevice->name() : "??");
    emit dataUpdated(this, m_DataValue);
}



