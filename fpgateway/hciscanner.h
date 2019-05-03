#ifndef HCISCANNER_H
#define HCISCANNER_H

#include <QObject>
#include <QProcess>

class HciScanner : public QObject
{
    Q_OBJECT
private:
    QProcess *m_HciToolProcess;

public:
    explicit HciScanner(QObject *parent = nullptr);
    ~HciScanner();

signals:
    void message(QString);

private slots:
    void readOutput();
    void readAll();

};

#endif // HCISCANNER_H
