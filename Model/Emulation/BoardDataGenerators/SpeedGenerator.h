#ifndef SPEEDGENERATOR_H
#define SPEEDGENERATOR_H

#include "ParameterGenerator.h"
#include <cmath>

class SpeedGenerator : public ParameterGenerator
{
    Q_OBJECT

public:
    explicit SpeedGenerator(QObject *parent = nullptr);
    ~SpeedGenerator() override = default;

    // ParameterGenerator interface
    BoardParameterSingle* generate(double time) override;
    QString getName() const override;

    // Настройка параметров генерации
    void setBaseSpeed(double baseSpeed);
    void setAmplitude(double amplitude);
    void setPeriod(double period);

private:
    double m_baseSpeed;         // Базовая скорость в м/с
    double m_amplitude;         // Амплитуда колебаний в м/с
    double m_period;            // Период колебаний в секундах
};

#endif // SPEEDGENERATOR_H
