#ifndef LONGITUDEGENERATOR_H
#define LONGITUDEGENERATOR_H

#include "ParameterGenerator.h"

class LongitudeGenerator : public ParameterGenerator
{
    Q_OBJECT

public:
    explicit LongitudeGenerator(QObject *parent = nullptr);
    ~LongitudeGenerator() override = default;

    // ParameterGenerator interface
    BoardParameterSingle* generate(double time) override;
    QString getName() const override;

    // Настройка параметров генерации
    void setStartLongitude(double startLongitude);
    void setSpeed(double speedDegreesPerSecond);

private:
    double m_startLongitude;    // Начальная долгота в градусах
    double m_speed;             // Скорость изменения в градусах в секунду
};

#endif // LONGITUDEGENERATOR_H
