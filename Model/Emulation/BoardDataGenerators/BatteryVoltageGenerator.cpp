#include "BatteryVoltageGenerator.h"
#include "./../../Parameters/BoardParameterSingle.h"
#include <cmath>
#include <QDateTime>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

BatteryVoltageGenerator::BatteryVoltageGenerator(QObject *parent)
    : ParameterGenerator(parent)
{
}

QString BatteryVoltageGenerator::getName() const
{
    return "telemetry.batteryVoltage";
}

QList<BoardParameterSingle*> BatteryVoltageGenerator::generate(double time)
{
    const double base_voltage = 24.0;
    const double amplitude = 3.0;
    const double period = 10.0;

    double voltage = base_voltage + amplitude * sin(2 * M_PI * time / period);
    
    BoardParameterSingle* p = new BoardParameterSingle(getName(), voltage, QDateTime::currentDateTime());
    return {p};
}
