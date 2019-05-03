#ifndef BLUEZSCANNER_H
#define BLUEZSCANNER_H

#include <QObject>

class BluezScanner : public QObject
{
    Q_OBJECT

private:
    int m_DeviceId;

public:
    explicit BluezScanner(QObject *parent = nullptr);

private:
    bool leScan(int dev_id);

signals:
    void message(QString);

public slots:
};

#endif // BLUEZSCANNER_H
