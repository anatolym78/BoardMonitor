#ifndef BOARDPARAMETERSINGLE_H
#define BOARDPARAMETERSINGLE_H

#include <QString>
#include <QVariant>
#include <QDateTime>
#include <QObject>
#include "BoardParameterValue.h"

// Класс для хранения одного параметра с одним значением
class BoardParameterSingle : public QObject
{
	Q_OBJECT
	Q_PROPERTY(QString label READ label WRITE setLabel NOTIFY labelChanged)
	Q_PROPERTY(QString unit READ unit WRITE setUnit NOTIFY unitChanged)
	Q_PROPERTY(QVariant value READ value WRITE setValue NOTIFY valueChanged)
	Q_PROPERTY(QDateTime timestamp READ timestamp WRITE setTimestamp NOTIFY timestampChanged)

public:
	explicit BoardParameterSingle(QObject *parent = nullptr);
	BoardParameterSingle(const QString &label, const QString &unit = QString(), QObject *parent = nullptr);
	BoardParameterSingle(const QString &label, const QVariant &value, const QString &unit = QString(), QObject *parent = nullptr);
	BoardParameterSingle(const QString &label, const QVariant &value, const QDateTime &timestamp, const QString &unit = QString(), QObject *parent = nullptr);
	
	// Геттеры
	QString label() const;
	bool isComplexLabel() const;
	QString label(int partIndex) const;
	int countLabelParts() const;
	QStringList labelParts() const;
	QVariant value() const;
	QVariantList values() const;
	int isArrayValue() const;
	int countValues() const;
	QVariant value(int arrayIndex) const;
	QString valueToString() const;
	QString unit() const { return m_unit; }
	QDateTime timestamp() const { return m_timestamp; }
	bool isValid() const;
	
	// Сеттеры
	void setLabel(const QString &label);
	void setUnit(const QString &unit);
	void setValue(const QVariant &value);
	void setTimestamp(const QDateTime &timestamp);
	
	// Создание BoardParameterValue из данного параметра
	BoardParameterValue* toParameterValue(QObject *parent = nullptr) const;
	
	// Операторы сравнения
	bool operator==(const BoardParameterSingle &other) const;
	bool operator!=(const BoardParameterSingle &other) const;

signals:
	void labelChanged();
	void unitChanged();
	void valueChanged();
	void timestampChanged();

private:
	QString m_label;           // Название параметра
	QString m_unit;            // Единица измерения
	QVariant m_value;          // Значение параметра
	QDateTime m_timestamp;     // Время получения параметра
};

#endif // BOARDPARAMETERSINGLE_H
