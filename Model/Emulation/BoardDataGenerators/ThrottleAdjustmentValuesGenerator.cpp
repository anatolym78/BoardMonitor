#include "ThrottleAdjustmentValuesGenerator.h"
#include <QVariantList>
#include "./../../../Model/Parameters/BoardParameterSingle.h"
#include <cmath>

ThrottleAdjustmentValuesGenerator::ThrottleAdjustmentValuesGenerator(QObject *parent)
    : ParameterGenerator(parent)
{
}

QList<BoardParameterSingle*> ThrottleAdjustmentValuesGenerator::generate(double time)
{
    const double period = 50.0;
    const double amplitude = 500.0;

    const double time_in_period = fmod(time, period);

    double value;
    if (time_in_period < period / 2.0)
    {
        value = -amplitude + (2.0 * amplitude / (period / 2.0)) * time_in_period;
    }
    else
    {
        value = amplitude - (2.0 * amplitude / (period / 2.0)) * (time_in_period - period / 2.0);
    }

    const int int_value = static_cast<int>(round(value));

    QVariantList values;
    values.append(-int_value); // motor 1 (right)
    values.append(-int_value); // motor 2 (right)
    values.append(int_value);  // motor 3 (left)
    values.append(int_value);  // motor 4 (left)

    BoardParameterSingle* p = new BoardParameterSingle("stabData.throttleAdjustmentValues", values, QDateTime::currentDateTime(), "", this);
    return {p};
}

QString ThrottleAdjustmentValuesGenerator::getName() const
{
    return "stabData.throttleAdjustmentValues";
}
