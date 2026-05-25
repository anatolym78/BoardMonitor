#ifndef STEP_H
#define STEP_H

#include <QVariant>

template<typename T>
class Step
{
public:
    Step() : m_step(T{}), m_hasStep(false) {}
    explicit Step(T step) : m_step(step), m_hasStep(true) {}

    T getStep() const { return m_step; }
    bool hasStep() const { return m_hasStep; }

    void setStep(T step)
    {
        m_step = step;
        m_hasStep = true;
    }

    void clearStep()
    {
        m_step = T{};
        m_hasStep = false;
    }

private:
    T m_step;
    bool m_hasStep;
};

#endif // STEP_H
