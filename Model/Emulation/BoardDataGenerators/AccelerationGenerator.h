#ifndef ACCELERATIONGENERATOR_H
#define ACCELERATIONGENERATOR_H

#include "ParameterGenerator.h"
#include "./../../Parameters/BoardParameterSingle.h"
#include <cmath>

class AccelerationGenerator : public ParameterGenerator
{
    Q_OBJECT

public:
    explicit AccelerationGenerator(QObject *parent = nullptr);
    QList<BoardParameterSingle*> generate(double time) override;
    QString getName() const override;
};

#endif // ACCELERATIONGENERATOR_H
