

#include <QDebug>
#include <QListWidget>
#include <QMessageBox>
#include <QTextStream>
#include <QFile>
#include <QVBoxLayout>
#include <QProcess>
#include <QTimer>
#include <QMessageBox>

#include "mainwindow.h"
#include "bledevicemanager.h"
#include "hciscanner.h"
#include "globals.h"
#include "devicedatadisplay.h"
#include "bledevice.h"


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      m_TcpSocket(nullptr)
{

    g_MainWindow = this;

    try
    {
        this->loadConfig();
    }
    catch(QString err)
    {
        QMessageBox::critical(nullptr, "Error in config file", err);
        qDebug() << err;
    }

    m_TcpSocket = new QTcpSocket(this);
    connect(m_TcpSocket, &QTcpSocket::connected, this,
            [=]()
            {
                this->showMessage("Connected to server");
            }
            );



    g_BleDeviceManager = new BleDeviceManager(this);
    connect(g_BleDeviceManager, &BleDeviceManager::message, this, &MainWindow::showMessage);
    connect(g_BleDeviceManager, &BleDeviceManager::deviceConnected, this, &MainWindow::deviceConnected);

    QWidget *w = new QWidget;
    QVBoxLayout *vl = new QVBoxLayout(w);

    QLabel *lbl_title = new QLabel;
    lbl_title->setText("Gateway");
    QFont fnt(lbl_title->font());
    fnt.setPointSize(24);
    fnt.setBold(true);
    lbl_title->setFont(fnt);
    lbl_title->setAlignment(Qt::AlignCenter);
    lbl_title->setFrameStyle(QFrame::Raised | QFrame::Panel);

    vl->addWidget(lbl_title, 1);

    m_DataLayout = new QGridLayout;

    vl->addLayout(m_DataLayout, 60);

    m_MsgList = new QListWidget;
    vl->addWidget(m_MsgList, 40);


    this->setCentralWidget(w);

    //HciScanner *scanner = new HciScanner(this);
    //connect(scanner, &HciScanner::message, this, &MainWindow::showMessage);

    this->resize(900, 700);
}




MainWindow::~MainWindow()
{



}



void MainWindow::loadConfig()
{
    QString read_line;
    bool ok;

    enum section_t { SECTION_INVALID, SECTION_FEATURES, SECTION_DEVICES, SECTION_OTHERS };
    section_t curr_section = SECTION_INVALID;

    QFile file("config.txt");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        throw QString("Error opening file");

    QTextStream stream(&file);
    while(!stream.atEnd())
    {
        QString line = stream.readLine().simplified();
        if(line.isEmpty() || line.startsWith('#')) continue;

        if(line.startsWith('['))
        {
            if(line == "[FEATURES]")
                curr_section = SECTION_FEATURES;
            else if(line == "[DEVICES]")
                curr_section = SECTION_DEVICES;
            else if(line == "[OTHERS]")
                curr_section = SECTION_OTHERS;
            continue;
        }

        QStringList split_parts = line.split(' ', QString::SkipEmptyParts);

        if(curr_section == SECTION_FEATURES)
        {
            if(split_parts.size() != 2) throw QString("Invalid format: " + read_line);

            IdFeature feature(split_parts[0], split_parts[1].toInt(&ok));
            if(!ok) throw QString("Invalid number: " + read_line);

            g_IdFeatures.push_back(feature);
        }
        else if(curr_section == SECTION_DEVICES)
        {
            if(split_parts.size() < 2) throw QString("Invalid format: " + read_line);

            KnownDevice dev(split_parts[0]);
            for(int i = 1; i < split_parts.size(); ++i)
            {
                QStringList feature_info = split_parts[i].split(',');
                if(feature_info.size() != 4) throw QString("Invalid format: " + read_line);
                IdFeature *feature = nullptr;
                for(int i = 0; i < g_IdFeatures.size(); ++i)
                {
                    if(g_IdFeatures[i].name() == feature_info[0])
                    {
                        feature = &g_IdFeatures[i];
                        break;
                    }
                }

                if(feature == nullptr) throw QString("Invalid feature name: " + read_line);

                dev.addFeature(feature,
                               feature_info[1].toUInt(), feature_info[2].toUInt(),
                               feature_info[3].toInt());
            }

            g_KnownDevices.push_back(dev);

        }
        else if(curr_section == SECTION_OTHERS)
        {
            if(split_parts[0] == "SERVER_IP")
            {
                ok = (split_parts.size()>=3) && m_ServerAddress.setAddress(split_parts[2]);
                if(!ok) throw QString("Invalid feature address: " + read_line);
            }
            if(split_parts[0] == "SERVER_PORT")
            {
                m_ServerPort = split_parts[2].toUInt(&ok) & 0x0ffff;
                if(!ok) throw QString("Invalid port number: " + read_line);
            }
        }


    }
}



void MainWindow::showMessage(QString msg)
{

    m_MsgList->addItem(msg);
}



void MainWindow::deviceConnected(BleDevice *device)
{

    for(int i = 0; i < m_DeviceDisplays.size(); ++i)
    {
        if(m_DeviceDisplays[i]->device() == device)
        {
            return;
        }
    }

    const int num_objects = m_DeviceDisplays.size();
    const int num_cols = 3;

    const int row = num_objects / num_cols;
    const int col = num_objects % num_cols;

    DeviceDataDisplay *ddd = new DeviceDataDisplay(device);
    m_DataLayout->addWidget(ddd, row, col, Qt::AlignCenter);

    m_DeviceDisplays.push_back(ddd);




}



void MainWindow::bleDeviceDataUpdated(BleDevice *device, int data)
{
    if(m_TcpSocket->state() == QAbstractSocket::UnconnectedState)
    {
        this->connectToServer();
    }
    else if(m_TcpSocket->state() == QAbstractSocket::ConnectedState)
    {
        if(device->identifiedDevice() != nullptr)
        {
            char tx_data[48];
            uint32_t data_val = static_cast<uint32_t>(data);
            tx_data[0] = char(0xab);
            tx_data[1] = char(0x12);
            tx_data[2] = char(0xcd);
            tx_data[3] = char(0x34);
            strncpy(tx_data+4,  device->identifiedDevice()->name().toLatin1().constData(), 20);
            strncpy(tx_data+24, device->bleDeviceInfo().name().toLatin1().constData(), 20);
            memcpy(tx_data+44, &data_val, sizeof(data_val));

            m_TcpSocket->write(tx_data, 48);
        }
    }
}



void MainWindow::connectToServer()
{
    QStringList args;
    args << "-n";
    args << m_ServerAddress.toString();

    QProcess::startDetached("fwknop", args);    //start port knock

    //wait for a few seconds before attempting connection
    QTimer::singleShot(1000, this,
                       [=]()
                       {
                            m_TcpSocket->connectToHost(m_ServerAddress, m_ServerPort);
                       }
                       );

}



void MainWindow::closeEvent(QCloseEvent *)
{
    if(m_TcpSocket->state() != QAbstractSocket::UnconnectedState)
    {
        m_TcpSocket->disconnectFromHost();
    }

    g_BleDeviceManager->shutdown();
}







