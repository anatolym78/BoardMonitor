#ifndef THROTTLEADJUSTMENTVALUESGENERATOR_H
#define THROTTLEADJUSTMENTVALUESGENERATOR_H

#include "ParameterGenerator.h"
#include "./../../Parameters/BoardParameterSingle.h"

class ThrottleAdjustmentValuesGenerator : public ParameterGenerator
{
    Q_OBJECT

public:
    explicit ThrottleAdjustmentValuesGenerator(QObject *parent = nullptr);
    QList<BoardParameterSingle*> generate(double time) override;
    QString getName() const override;
};

#endif // THROTTLEADJUSTMENTVALUESGENERATOR_H
