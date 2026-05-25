#ifndef LATITUDEGENERATOR_H
#define LATITUDEGENERATOR_H

#include "ParameterGenerator.h"

class LatitudeGenerator : public ParameterGenerator
{
    Q_OBJECT

public:
    explicit LatitudeGenerator(QObject *parent = nullptr);
    ~LatitudeGenerator() override = default;

    // ParameterGenerator interface
    BoardParameterSingle* generate(double time) override;
    QString getName() const override;

    // Настройка параметров генерации
    void setStartLatitude(double startLatitude);
    void setSpeed(double speedDegreesPerSecond);

private:
    double m_startLatitude;     // Начальная широта в градусах
    double m_speed;             // Скорость изменения в градусах в секунду
};

#endif // LATITUDEGENERATOR_H
