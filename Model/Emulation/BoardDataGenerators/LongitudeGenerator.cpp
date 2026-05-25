#include "LongitudeGenerator.h"
#include <QDebug>
#include "./../../Parameters/BoardParameterSingle.h"

LongitudeGenerator::LongitudeGenerator(QObject *parent)
    : ParameterGenerator(parent)
    , m_startLongitude(37.6173)  // Начальная долгота (Москва)
    , m_speed(0.001)             // Скорость изменения (медленное движение)
{
}

BoardParameterSingle* LongitudeGenerator::generate(double time)
{
    // Генерируем долготу по линейному закону
    double longitude = m_startLongitude + m_speed * time;
    
    // Создаем параметр с названием, значением и единицей измерения
    BoardParameterSingle *param = new BoardParameterSingle(tr("Longitude"), longitude, QDateTime::currentDateTime(), tr("°"));
    
    return param;
}

QString LongitudeGenerator::getName() const
{
    return "LongitudeGenerator";
}

void LongitudeGenerator::setStartLongitude(double startLongitude)
{
    m_startLongitude = startLongitude;
}

void LongitudeGenerator::setSpeed(double speedDegreesPerSecond)
{
    m_speed = speedDegreesPerSecond;
}
