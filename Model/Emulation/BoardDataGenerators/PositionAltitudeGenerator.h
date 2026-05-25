#ifndef POSITIONALTITUDEGENERATOR_H
#define POSITIONALTITUDEGENERATOR_H

#include "ParameterGenerator.h"

class PositionAltitudeGenerator : public ParameterGenerator
{
    Q_OBJECT

public:
    explicit PositionAltitudeGenerator(QObject *parent = nullptr);
    QList<BoardParameterSingle*> generate(double time) override;
    QString getName() const override;
};

#endif // POSITIONALTITUDEGENERATOR_H
