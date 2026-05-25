#ifndef GYROANGLERATESGENERATOR_H
#define GYROANGLERATESGENERATOR_H

#include "ParameterGenerator.h"

class GyroAngleRatesGenerator : public ParameterGenerator
{
    Q_OBJECT

public:
    explicit GyroAngleRatesGenerator(QObject *parent = nullptr);
    QList<BoardParameterSingle*> generate(double time) override;
    QString getName() const override;
};

#endif // GYROANGLERATESGENERATOR_H
