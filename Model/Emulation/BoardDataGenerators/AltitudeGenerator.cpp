#include "AltitudeGenerator.h"
#include <QDebug>
#include "./../../Parameters/BoardParameterSingle.h"
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

AltitudeGenerator::AltitudeGenerator(QObject *parent)
    : ParameterGenerator(parent)
    , m_baseAltitude(0)
    , m_amplitude(500)
    , m_period(5.0)
{
}

BoardParameterSingle* AltitudeGenerator::generate(double time)
{
    // Генерируем высоту по синусоидальному закону
    double altitude = m_baseAltitude + m_amplitude * sin(2.0 * M_PI * time / m_period);
    
    // Создаем параметр с названием, значением и единицей измерения
    BoardParameterSingle *param = new BoardParameterSingle(tr("Altitude"), altitude, QDateTime::currentDateTime(), tr("m"));
    
    return param;
}

QString AltitudeGenerator::getName() const
{
    return "AltitudeGenerator";
}

void AltitudeGenerator::setBaseAltitude(double baseAltitude)
{
    m_baseAltitude = baseAltitude;
}

void AltitudeGenerator::setAmplitude(double amplitude)
{
    m_amplitude = amplitude;
}

void AltitudeGenerator::setPeriod(double period)
{
    m_period = period;
}
