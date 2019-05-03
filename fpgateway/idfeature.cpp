
#include <QDebug>

#include "idfeature.h"


IdFeature::IdFeature(QString name, int index)
    : m_Name(name),
      m_Index(index)
{

}


IdFeature::~IdFeature()
{

}



QString IdFeature::name() const
{
    return m_Name;
}

void IdFeature::setName(const QString &Name)
{
    m_Name = Name;
}



bool IdFeature::getValue(const QByteArray &data, uint32_t *actual_value, uint32_t *norm_value)
{
    uint32_t value;
    if(m_Index >= data.size()) return false;

    memcpy(&value, data.data() + m_Index, sizeof(value));

    if(actual_value)
        *actual_value = value;

    if(norm_value)
        *norm_value = value;    //TODO - normalise

    qDebug() << "Feature: " << m_Name << " Value: " << value;

    return true;
}




