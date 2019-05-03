#ifndef IDFEATURE_H
#define IDFEATURE_H

#include <QString>

class IdFeature
{
private:
        QString   m_Name;
        int       m_Index;

public:
    IdFeature(QString name, int index);
    virtual ~IdFeature();
    QString name() const;
    void setName(const QString &Name);

    virtual bool getValue(const QByteArray& data, uint32_t *actual_value, uint32_t *norm_value);

};

#endif // IDFEATURE_H
