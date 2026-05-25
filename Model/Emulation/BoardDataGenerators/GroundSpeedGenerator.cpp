#include "GroundSpeedGenerator.h"

GroundSpeedGenerator::GroundSpeedGenerator(QObject *parent)
    : ParameterGenerator(parent)
    , m_averageSpeed(50.0)
    , m_amplitude(3.0)
    , m_period(10.0)
{
}

QString GroundSpeedGenerator::getName() const
{
    return "telemetry.groundSpeed";
}

QList<BoardParameterSingle*> GroundSpeedGenerator::generate(double time)
{
    double value = m_averageSpeed + m_amplitude * std::sin(2 * M_PI * time / m_period);
    int int_value = static_cast<int>(round(value));
    BoardParameterSingle* p = new BoardParameterSingle(getName(), QVariant::fromValue(int_value));
    return {p};
}
