#include "LatitudeGenerator.h"
#include <QDebug>
#include "./../../Parameters/BoardParameterSingle.h"

LatitudeGenerator::LatitudeGenerator(QObject *parent)
    : ParameterGenerator(parent)
    , m_startLatitude(0)  // Начальная широта (Москва)
    , m_speed(0.0005)           // Скорость изменения (медленное движение)
{
}

BoardParameterSingle* LatitudeGenerator::generate(double time)
{
    // Генерируем широту по линейному закону
    double latitude = m_startLatitude + m_speed * time;
    
    // Создаем параметр с названием, значением и единицей измерения
    BoardParameterSingle *param = new BoardParameterSingle(tr("Latitude"), latitude, QDateTime::currentDateTime(), tr("°"));
    
    return param;
}

QString LatitudeGenerator::getName() const
{
    return "LatitudeGenerator";
}

void LatitudeGenerator::setStartLatitude(double startLatitude)
{
    m_startLatitude = startLatitude;
}

void LatitudeGenerator::setSpeed(double speedDegreesPerSecond)
{
    m_speed = speedDegreesPerSecond;
}
