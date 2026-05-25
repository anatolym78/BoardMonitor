#include "IsArmedGenerator.h"
#include "./../../Parameters/BoardParameterSingle.h"
#include <cmath>
#include <QDateTime>

IsArmedGenerator::IsArmedGenerator(QObject *parent)
    : ParameterGenerator(parent)
{
}

QString IsArmedGenerator::getName() const
{
    return "stabData.isArmed";
}

QList<BoardParameterSingle*> IsArmedGenerator::generate(double time)
{
    const double period = 20.0;
    bool is_armed = fmod(time, period) < (period / 2.0);
    
    BoardParameterSingle* p = new BoardParameterSingle(getName(), is_armed, QDateTime::currentDateTime());
    return {p};
}
