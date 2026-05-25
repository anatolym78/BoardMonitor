#ifndef BATTERYVOLTAGEGENERATOR_H
#define BATTERYVOLTAGEGENERATOR_H

#include "ParameterGenerator.h"

class BatteryVoltageGenerator : public ParameterGenerator
{
    Q_OBJECT

public:
    explicit BatteryVoltageGenerator(QObject *parent = nullptr);
    QList<BoardParameterSingle*> generate(double time) override;
    QString getName() const override;
};

#endif // BATTERYVOLTAGEGENERATOR_H
