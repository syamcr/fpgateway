
#include "knowndevice.h"



KnownDevice::KnownDevice(QString name)
    : m_Name(name)
{

}


QString KnownDevice::name() const
{
    return m_Name;
}


QList<KnownDevice::FeatureValue> KnownDevice::features() const
{
    return m_Features;
}



void KnownDevice::addFeature(IdFeature *feature, uint32_t range_start, uint32_t range_end, int weight)
{
    FeatureValue fv;
    fv.feature = feature;
    fv.range_start = range_start;
    fv.range_end   = range_end;
    fv.weight      = weight;

    m_Features.push_back(fv);
}



int KnownDevice::getMatchScore(const QByteArray &data) const
{
    int score = 0;
    uint32_t actual_value, norm_value;
    bool ok;

    for(int i = 0; i < m_Features.size(); ++i)
    {
        const FeatureValue& fv = m_Features[i];
        if(fv.feature == nullptr) continue;

        ok = fv.feature->getValue(data, &actual_value, &norm_value);
        if(!ok) continue;

        if(norm_value >= fv.range_start && norm_value <= fv.range_end)
            score += fv.weight;

    }

    return score;
}



