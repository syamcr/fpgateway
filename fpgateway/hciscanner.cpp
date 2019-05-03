
#include <QDebug>
#include <QStringList>
#include <QTimer>

#include "hciscanner.h"

HciScanner::HciScanner(QObject *parent) : QObject(parent)
{

    m_HciToolProcess = new QProcess(this);

    connect(m_HciToolProcess, &QProcess::readyReadStandardOutput,
            this, &HciScanner::readOutput);

    m_HciToolProcess->setProcessChannelMode(QProcess::MergedChannels);
    QStringList args;
    args << "lescan";

    m_HciToolProcess->start("hcitool", args);

    QTimer *tmr = new QTimer;
    connect(tmr, &QTimer::timeout, this, &HciScanner::readAll);
    tmr->start(1000);
}



HciScanner::~HciScanner()
{
    m_HciToolProcess->terminate();
    m_HciToolProcess->waitForFinished(5000);
}



void HciScanner::readOutput()
{
    qDebug() << "readOutput";
    if(!m_HciToolProcess->canReadLine()) return;

    const int max_len = 100;
    char str_line[max_len+1];
    qint64 read_len;

    do
    {
        read_len = m_HciToolProcess->readLine(str_line, max_len);
        emit message(str_line);

        qDebug() << "Read line: " << str_line;
    } while(read_len > 0);

}

void HciScanner::readAll()
{
    qDebug() << "readAll";
    QByteArray b = m_HciToolProcess->readAllStandardOutput();
    qDebug() << QString(b);
    b = m_HciToolProcess->readAllStandardError();
    qDebug() << QString(b);
}


