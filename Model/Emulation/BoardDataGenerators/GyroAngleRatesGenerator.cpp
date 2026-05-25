#include "GyroAngleRatesGenerator.h"
#include "./../../Parameters/BoardParameterSingle.h"
#include <QVariantList>
#include <cmath>
#include <QDateTime>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

GyroAngleRatesGenerator::GyroAngleRatesGenerator(QObject *parent)
    : ParameterGenerator(parent)
{
}

QString GyroAngleRatesGenerator::getName() const
{
    return "stabData.gyroAngleRates";
}

QList<BoardParameterSingle*> GyroAngleRatesGenerator::generate(double time)
{
    const double period = 15.0;
    const double amplitude = 180.0;
    
    double v1 = amplitude * sin(2 * M_PI * time / period);
    double v2 = amplitude * cos(2 * M_PI * time / period);
    double v3 = amplitude * sin(2 * M_PI * time / (period / 2.0)); // Faster rate for variety

    QVariantList values;
    values.append(v1);
    values.append(v2);
    values.append(v3);
    
    BoardParameterSingle* p = new BoardParameterSingle(getName(), values, QDateTime::currentDateTime());
    return {p};
}
