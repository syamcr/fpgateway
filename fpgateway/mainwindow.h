#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGridLayout>
#include <QListWidget>
#include <QList>
#include <QTcpSocket>
#include <QHostAddress>

class BleDeviceManager;
class BleDevice;
class DeviceDataDisplay;

class MainWindow : public QMainWindow
{
    Q_OBJECT

private:
    QGridLayout *m_DataLayout;
    QListWidget *m_MsgList;
    QList<DeviceDataDisplay*> m_DeviceDisplays;

    QTcpSocket   *m_TcpSocket;
    QHostAddress  m_ServerAddress;
    uint16_t      m_ServerPort;


public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void loadConfig();

public slots:
    void bleDeviceDataUpdated(BleDevice *device, int data);
    void connectToServer();


private slots:
    void showMessage(QString msg);
    void deviceConnected(BleDevice *device);


protected:
    void closeEvent(QCloseEvent *event);

};

#endif // MAINWINDOW_H
