#ifndef GPSLOCATIONGENERATOR_H
#define GPSLOCATIONGENERATOR_H

#include "ParameterGenerator.h"

class GpsLocationGenerator : public ParameterGenerator
{
    Q_OBJECT

public:
    explicit GpsLocationGenerator(QObject *parent = nullptr);
    QList<BoardParameterSingle*> generate(double time) override;
    QString getName() const override;
};

#endif // GPSLOCATIONGENERATOR_H
