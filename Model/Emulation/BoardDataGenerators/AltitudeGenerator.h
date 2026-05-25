#ifndef ALTITUDEGENERATOR_H
#define ALTITUDEGENERATOR_H

#include "ParameterGenerator.h"
#include <cmath>

class AltitudeGenerator : public ParameterGenerator
{
    Q_OBJECT

public:
    explicit AltitudeGenerator(QObject *parent = nullptr);
    ~AltitudeGenerator() override = default;

    // ParameterGenerator interface
    BoardParameterSingle* generate(double time) override;
    QString getName() const override;

    // Настройка параметров генерации
    void setBaseAltitude(double baseAltitude);
    void setAmplitude(double amplitude);
    void setPeriod(double period);

private:
    double m_baseAltitude;      // Базовая высота в метрах
    double m_amplitude;         // Амплитуда колебаний в метрах
    double m_period;            // Период колебаний в секундах
};

#endif // ALTITUDEGENERATOR_H
