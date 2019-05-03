#ifndef BLEDEVICE_H
#define BLEDEVICE_H

#include <QObject>
#include <QBluetoothDeviceInfo>
#include <QLowEnergyController>
#include <QTimer>

#include "idfeature.h"
#include "knowndevice.h"

class BleDevice : public QObject
{
    Q_OBJECT

    friend class BleDeviceManager;

private:
    QBluetoothDeviceInfo     m_DeviceInfo;
    QLowEnergyController    *m_Controller;
    QLowEnergyService       *m_DataService;
    QLowEnergyService       *m_IdService;
    QLowEnergyCharacteristic m_DataChar;
    QLowEnergyCharacteristic m_IdChar;
    QByteArray               m_FpidValue;
    int                      m_DataValue;

    KnownDevice             *m_IdentifiedDevice;
    static QTimer           *m_ConnectionTimer;
    static int               m_IdentificationScoreThreshold;

    explicit BleDevice(const QBluetoothDeviceInfo& dev_info, QObject *parent = nullptr);
    ~BleDevice();

    void disconnect();

signals:
    void message(QString);
    void dataUpdated(BleDevice *device, int data);
    void deviceConnected(BleDevice *device);
    void deviceIdentificationChanged(BleDevice *device);

public:
    KnownDevice* identifiedDevice() { return m_IdentifiedDevice; }

    void setIdentifiedDevice(KnownDevice *device)
    {
        m_IdentifiedDevice = device;
        emit deviceIdentificationChanged(this);
    }

    const QBluetoothDeviceInfo& bleDeviceInfo() const { return m_DeviceInfo; }
    const QByteArray& fpIdValue() const { return  m_FpidValue; }

    static int idScoreThreshold();
    static void setIdScoreThreshold(int score);

private slots:
    void serviceScanFinished();
    void connectionError(QLowEnergyController::Error err);
    void connectionSuccessful();

    void attemptConnection();

    void dataServiceDetailsDiscoveryFinished(QLowEnergyService::ServiceState new_state);
    void idServiceDetailsDiscoveryFinished(QLowEnergyService::ServiceState new_state);

    void fpidValueRead(const QLowEnergyCharacteristic &characteristic, const QByteArray &value);
    void dataValueUpdateHandler(const QLowEnergyCharacteristic &c, const QByteArray &value);


};

#endif // BLEDEVICE_H
