#ifndef GROUNDSPEEDGENERATOR_H
#define GROUNDSPEEDGENERATOR_H

#include "ParameterGenerator.h"
#include "./../../Parameters/BoardParameterSingle.h"
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

class GroundSpeedGenerator : public ParameterGenerator
{
    Q_OBJECT

public:
    explicit GroundSpeedGenerator(QObject *parent = nullptr);
    QList<BoardParameterSingle*> generate(double time) override;
    QString getName() const override;

private:
    double m_averageSpeed;
    double m_amplitude;
    double m_period;
};

#endif // GROUNDSPEEDGENERATOR_H
