#ifndef ISARMEDGENERATOR_H
#define ISARMEDGENERATOR_H

#include "ParameterGenerator.h"

class IsArmedGenerator : public ParameterGenerator
{
    Q_OBJECT

public:
    explicit IsArmedGenerator(QObject *parent = nullptr);
    QList<BoardParameterSingle*> generate(double time) override;
    QString getName() const override;
};

#endif // ISARMEDGENERATOR_H
