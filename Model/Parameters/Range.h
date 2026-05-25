#ifndef RANGE_H
#define RANGE_H

#include <QVariant>

template<typename T>
class Range
{
public:
    Range() : m_minimum(T{}), m_maximum(T{}), m_hasRange(false) {}
    Range(T minimum, T maximum) : m_minimum(minimum), m_maximum(maximum), m_hasRange(true) {}

    T getMinimum() const { return m_minimum; }
    T getMaximum() const { return m_maximum; }
    bool hasRange() const { return m_hasRange; }

    void setRange(T minimum, T maximum)
    {
        m_minimum = minimum;
        m_maximum = maximum;
        m_hasRange = true;
    }

    void clearRange()
    {
        m_minimum = T{};
        m_maximum = T{};
        m_hasRange = false;
    }

private:
    T m_minimum;
    T m_maximum;
    bool m_hasRange;
};

#endif // RANGE_H
