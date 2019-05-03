#ifndef KNOWNDEVICE_H
#define KNOWNDEVICE_H

#include <QList>

#include <idfeature.h>

class KnownDevice
{
public:
    struct FeatureValue
    {
        IdFeature *feature;
        uint32_t range_start;
        uint32_t range_end;
        int      weight;
    };

private:
    QString m_Name;
    QList<FeatureValue> m_Features;

public:
    KnownDevice(QString name);
    int getMatchScore(const QByteArray& data) const;

    QString name() const;
    void addFeature(IdFeature *feature, uint32_t range_start, uint32_t range_end, int weight);
    QList<FeatureValue> features() const;
};




#endif // KNOWNDEVICE_H
