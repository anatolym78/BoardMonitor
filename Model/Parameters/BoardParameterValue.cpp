#include "BoardParameterValue.h"

BoardParameterValue::BoardParameterValue(QObject *parent)
    : QObject(parent)
    , m_timestamp(QDateTime::currentDateTime())
{
}

BoardParameterValue::BoardParameterValue(const QVariant &value, const QDateTime &timestamp, QObject *parent)
    : QObject(parent)
    , m_value(value)
    , m_timestamp(timestamp)
{
}

void BoardParameterValue::setValue(const QVariant &value)
{
    if (m_value != value) {
        m_value = value;
        emit valueChanged();
    }
}

void BoardParameterValue::setTimestamp(const QDateTime &timestamp)
{
    if (m_timestamp != timestamp) {
        m_timestamp = timestamp;
        emit timestampChanged();
    }
}
