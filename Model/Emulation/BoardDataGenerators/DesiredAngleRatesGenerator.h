#ifndef DESIREDANGLERATESGENERATOR_H
#define DESIREDANGLERATESGENERATOR_H

#include "ParameterGenerator.h"
#include "./../../Parameters/BoardParameterSingle.h"
#include <cmath>

class DesiredAngleRatesGenerator : public ParameterGenerator
{
    Q_OBJECT

public:
    explicit DesiredAngleRatesGenerator(QObject *parent = nullptr);
    QList<BoardParameterSingle*> generate(double time) override;
    QString getName() const override;
};

#endif // DESIREDANGLERATESGENERATOR_H
