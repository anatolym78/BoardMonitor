#ifndef PARAMETERGENERATOR_H
#define PARAMETERGENERATOR_H

#include <QObject>
#include <QList>
#include "./../../Parameters/BoardParameterSingle.h"

class ParameterGenerator : public QObject
{
    Q_OBJECT

public:
    explicit ParameterGenerator(QObject *parent = nullptr);
    virtual ~ParameterGenerator() = default;

    // Абстрактная функция генерации параметра
    virtual QList<BoardParameterSingle*> generate(double time) = 0;

    // Получение названия генератора
    virtual QString getName() const = 0;
};

#endif // PARAMETERGENERATOR_H
