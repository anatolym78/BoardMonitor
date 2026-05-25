#include "AccelerationGenerator.h"
#include <QVariantList>
#include <QDateTime>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

AccelerationGenerator::AccelerationGenerator(QObject *parent)
    : ParameterGenerator(parent)
{
}

QString AccelerationGenerator::getName() const
{
    return "telemetry.acceleration";
}

QList<BoardParameterSingle*> AccelerationGenerator::generate(double time)
{
    const double period = 10.0;
    const double amplitude = 3.0;

    const double time_in_period = fmod(time, period);
    double value;
    if (time_in_period < period / 2.0)
    {
        value = (amplitude / (period / 2.0)) * time_in_period;
    }
    else
    {
        value = amplitude - (amplitude / (period / 2.0)) * (time_in_period - period / 2.0);
    }
    
    QVariantList values;
    values.append(value);
    values.append(value);
    values.append(value);

    BoardParameterSingle* p = new BoardParameterSingle(getName(), values, QDateTime::currentDateTime());
    return {p};
}
