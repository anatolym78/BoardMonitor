#include "BoardParameterSingle.h"

BoardParameterSingle::BoardParameterSingle(QObject *parent)
	: QObject(parent)
	, m_timestamp(QDateTime::currentDateTime())
{
}

BoardParameterSingle::BoardParameterSingle(const QString &label, const QString &unit, QObject *parent)
	: QObject(parent)
	, m_label(label)
	, m_unit(unit)
	, m_timestamp(QDateTime::currentDateTime())
{
}

BoardParameterSingle::BoardParameterSingle(const QString &label, const QVariant &value, const QString &unit, QObject *parent)
	: QObject(parent)
	, m_label(label)
	, m_unit(unit)
	, m_value(value)
	, m_timestamp(QDateTime::currentDateTime())
{
}

BoardParameterSingle::BoardParameterSingle(const QString &label, const QVariant &value, const QDateTime &timestamp, const QString &unit, QObject *parent)
	: QObject(parent)
	, m_label(label)
	, m_unit(unit)
	, m_value(value)
	, m_timestamp(timestamp)
{
}

QString BoardParameterSingle::label() const
{
	return m_label;
}

QString BoardParameterSingle::label(int partIndex) const
{
	if (partIndex < 0 || partIndex >= countLabelParts()) return QString();

	return labelParts()[partIndex];
}

bool BoardParameterSingle::isComplexLabel() const
{
	return countLabelParts() > 1;
}

int BoardParameterSingle::countLabelParts() const
{
	return labelParts().count();
}

QStringList BoardParameterSingle::labelParts() const
{
	return m_label.split('.');
}

QVariant BoardParameterSingle::value() const
{
	return m_value;
}

QVariant BoardParameterSingle::value(int arrayIndex) const
{
	if (!isArrayValue()) return m_value;

	auto list = m_value.toList();
	
	if (arrayIndex < 0 || arrayIndex >= list.count()) return QVariant();

	return list.at(arrayIndex);
}

QVariantList BoardParameterSingle::values() const
{
	if (isArrayValue())
	{
		return m_value.toList();
	}

	return QVariantList()<<m_value;
}

QString BoardParameterSingle::valueToString() const
{
	if (isArrayValue())
	{
		QStringList stringList;
		for (auto& val : m_value.toList())
		{
			stringList << val.toString();
		}

		return stringList.join(';');
	}
	else
	{
		return m_value.toString();
	}
}

int BoardParameterSingle::isArrayValue() const
{
	return m_value.canConvert<QVariantList>();
}

int BoardParameterSingle::countValues() const
{
	if (isArrayValue())
	{
		return values().count();
	}

	return 1;
}

bool BoardParameterSingle::isValid() const
{
	if (m_label.isEmpty()) return false;

	if (!m_value.isValid()) return false;

	return true;
}

void BoardParameterSingle::setLabel(const QString &label)
{
	if (m_label != label) 
	{
		m_label = label;
		emit labelChanged();
	}
}

void BoardParameterSingle::setValue(const QVariant &value)
{
	if (m_value != value) 
	{
		m_value = value;
		emit valueChanged();
	}
}

void BoardParameterSingle::setUnit(const QString &unit)
{
	if (m_unit != unit) 
	{
		m_unit = unit;
		emit unitChanged();
	}
}

void BoardParameterSingle::setTimestamp(const QDateTime &timestamp)
{
	if (m_timestamp != timestamp) 
	{
		m_timestamp = timestamp;
		emit timestampChanged();
	}
}

BoardParameterValue* BoardParameterSingle::toParameterValue(QObject *parent) const
{
	return new BoardParameterValue(m_value, m_timestamp, parent);
}

bool BoardParameterSingle::operator==(const BoardParameterSingle &other) const
{
	return m_label == other.m_label && 
		   m_unit == other.m_unit && 
		   m_value == other.m_value && 
		   m_timestamp == other.m_timestamp;
}

bool BoardParameterSingle::operator!=(const BoardParameterSingle &other) const
{
	return !(*this == other);
}
