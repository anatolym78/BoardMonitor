#ifndef BOARDPARAMETERVALUE_H
#define BOARDPARAMETERVALUE_H

#include <QVariant>
#include <QDateTime>
#include <QObject>

// Класс для хранения значения параметра и времени его получения
class BoardParameterValue : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QVariant value READ value WRITE setValue NOTIFY valueChanged)
    Q_PROPERTY(QDateTime timestamp READ timestamp WRITE setTimestamp NOTIFY timestampChanged)

public:
    explicit BoardParameterValue(QObject *parent = nullptr);
    BoardParameterValue(const QVariant &value, const QDateTime &timestamp = QDateTime::currentDateTime(), QObject *parent = nullptr);
    
    // Геттеры
    QVariant value() const { return m_value; }
    QDateTime timestamp() const { return m_timestamp; }
    
    // Сеттеры
    void setValue(const QVariant &value);
    void setTimestamp(const QDateTime &timestamp);
    
    // Операторы сравнения
    bool operator==(const BoardParameterValue &other) const {
        return m_value == other.m_value && m_timestamp == other.m_timestamp;
    }
    
    bool operator!=(const BoardParameterValue &other) const {
        return !(*this == other);
    }

signals:
    void valueChanged();
    void timestampChanged();

private:
    QVariant m_value;          // Значение параметра
    QDateTime m_timestamp;     // Время получения параметра
};

#endif // BOARDPARAMETERVALUE_H
