#include "PositionAltitudeGenerator.h"
#include "./../../Parameters/BoardParameterSingle.h"
#include <QVariantList>
#include <cmath>
#include <QDateTime>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

PositionAltitudeGenerator::PositionAltitudeGenerator(QObject *parent)
    : ParameterGenerator(parent)
{
}

QString PositionAltitudeGenerator::getName() const
{
    return "telemetry.absolutePosition"; // Основное имя
}

QList<BoardParameterSingle*> PositionAltitudeGenerator::generate(double time)
{
    // Генерация для absolutePosition
    const double x_period = 30.0;
    const double x_amplitude = 100.0;
    double x = x_amplitude * sin(2 * M_PI * time / x_period);

    const double y_period = 45.0;
    const double y_amplitude = 150.0;
    double y = y_amplitude * cos(2 * M_PI * time / y_period); // cos для разнообразия

    const double z_period = 60.0;
    const double z_amplitude = 50.0;
    const double z_offset = 60.0;
    double z = z_offset + z_amplitude * sin(2 * M_PI * time / z_period);

    QVariantList pos_values;
    pos_values.append(x);
    pos_values.append(y);
    pos_values.append(z);
    BoardParameterSingle* posParam = new BoardParameterSingle(getName(), pos_values, QDateTime::currentDateTime());

    // Генерация для altitude
    BoardParameterSingle* altParam = new BoardParameterSingle("telemetry.altitude", z, QDateTime::currentDateTime());
    
    return {posParam, altParam};
}
