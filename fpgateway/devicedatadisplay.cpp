
#include <QSpacerItem>

#include "devicedatadisplay.h"
#include "globals.h"



DeviceDataDisplay::DeviceDataDisplay(BleDevice *device, QWidget *parent)
    : QFrame(parent),
      m_Device(device)
{

    this->setFrameStyle(QFrame::Panel | QFrame::Raised);

    QFont f = this->font();
    f.setFamily("DejaVu Sans Mono");
    this->setFont(f);

    m_GridLayout = new QGridLayout(this);

    m_lblTitle = new QLabel;
    m_lblNameTag = new QLabel;
    m_lblName = new QLabel;
    m_lblDataValue = new QLabel;

    m_GridLayout->addWidget(m_lblTitle,     0, 0, 1, 2);
    m_GridLayout->addWidget(m_lblNameTag,   1, 0, 1, 1);
    m_GridLayout->addWidget(m_lblName,      1, 1, 1, 1);
    m_GridLayout->addWidget(m_lblDataValue, 2, 0, 1, 2);

    m_GridLayout->setRowStretch(0, 2);
    m_GridLayout->setRowStretch(1, 1);
    m_GridLayout->setRowStretch(2, 4);

    m_lblTitle->setText("<UNKNOWN>");
    m_lblTitle->setFrameStyle(QFrame::Panel | QFrame::Raised);
    m_lblTitle->setAlignment(Qt::AlignCenter);

    QFont fnt = m_lblTitle->font();
    fnt.setPointSize(20);
    fnt.setBold(true);
    m_lblTitle->setFont(fnt);

    m_lblNameTag->setText("Advertised as:");
    m_lblNameTag->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
    fnt = m_lblNameTag->font();
    fnt.setPointSize(12);
    m_lblNameTag->setFont(fnt);

    m_lblName->setFont(fnt);
    m_lblName->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);

    m_lblDataValue->setAlignment(Qt::AlignCenter);
    fnt = m_lblDataValue->font();
    fnt.setPointSize(32);
    m_lblDataValue->setFont(fnt);


    m_GridLayout->setMargin(1);

    setBackgroundColour(this, "#ffdddd");


    if(m_Device != nullptr)
    {
        connect(m_Device, &BleDevice::dataUpdated, this, &DeviceDataDisplay::dataUpdated);

        connect(m_Device, &BleDevice::deviceIdentificationChanged, this, &DeviceDataDisplay::deviceIdentificationChanged);

        m_lblName->setText(m_Device->bleDeviceInfo().name());

        for(int i = 0; i < g_IdFeatures.size(); ++i)
        {
            IdFeature &feature = g_IdFeatures[i];
            QLabel *lbl_ftr = new QLabel( QString("%1 : %2").arg(feature.name(), -5).arg("?") );
            m_GridLayout->addWidget(lbl_ftr,   3+i, 0, 1, 2); lbl_ftr->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);

            m_GridLayout->setRowStretch(3+i, 1);
        }


    }

    this->setFixedSize(250, 200);
}


BleDevice *DeviceDataDisplay::device()
{
    return m_Device;
}


void DeviceDataDisplay::dataUpdated(BleDevice *, int data)
{
    m_lblDataValue->setText( QString::number(data) );
}



void DeviceDataDisplay::deviceIdentificationChanged(BleDevice *device)
{
    if(device->identifiedDevice())
    {
        m_lblTitle->setText(device->identifiedDevice()->name());
        setBackgroundColour(this, "#9bff94");
    }
    else
    {
        m_lblTitle->setText("<UNKNOWN>");
        setBackgroundColour(this, "#ffdddd");
    }

    const QByteArray& fpdata = device->fpIdValue();
    uint32_t val;

    for(int i = 0; i < g_IdFeatures.size(); ++i)
    {
        IdFeature &feature = g_IdFeatures[i];
        QLabel *lbl_value = static_cast<QLabel*>(m_GridLayout->itemAtPosition(3+i, 0)->widget());

        if(fpdata.isEmpty())
        {
            lbl_value->setText( QString("%1 : %2").arg(feature.name(), -5).arg("?") );
        }
        else
        {
            feature.getValue(fpdata, &val, nullptr);
            lbl_value->setText( QString("%1 : %2").arg(feature.name(), -5).arg(val) );
        }

    }

}


