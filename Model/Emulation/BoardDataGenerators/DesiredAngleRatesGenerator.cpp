#include "DesiredAngleRatesGenerator.h"
#include <QVariantList>
#include <QDateTime>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

DesiredAngleRatesGenerator::DesiredAngleRatesGenerator(QObject *parent)
    : ParameterGenerator(parent)
{
}

QString DesiredAngleRatesGenerator::getName() const
{
    return "stabData.desiredAngleRates";
}

QList<BoardParameterSingle*> DesiredAngleRatesGenerator::generate(double time)
{
    const double period = 20.0;
    const double amplitude = 90.0;
    
    double v1 = amplitude * sin(2 * M_PI * time / period);
    double v2 = amplitude * sin(2 * M_PI * time / period + (2 * M_PI / 3));
    double v3 = amplitude * sin(2 * M_PI * time / period + (4 * M_PI / 3));

    QVariantList values;
    values.append(v1);
    values.append(v2);
    values.append(v3);

    BoardParameterSingle* p = new BoardParameterSingle(getName(), values, QDateTime::currentDateTime());
    return {p};
}
