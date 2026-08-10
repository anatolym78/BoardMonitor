#include "ParameterTreeHistoryItem.h"
#include "ParameterTreeArrayItem.h"
#include <algorithm>
#include <random>
#include <QColor>

ParameterTreeHistoryItem::ParameterTreeHistoryItem(const QString &label, ParameterTreeItem *parent)
	: ParameterTreeItem(label, parent)
{
	// Генерируем цвет для параметра
	if (parent && parent->type() == ItemType::Array)
	{
		// Если родитель - массив, используем hue от массива и разные saturation
		auto arrayItem = static_cast<ParameterTreeArrayItem*>(parent);
		int hue = arrayItem->arrayHue();
		int saturationIndex = arrayItem->getNextSaturationIndex();
		
		// Генерируем saturation в диапазоне [192, 255]
		// Распределяем saturation равномерно для разных элементов массива
		static std::random_device rd;
		static std::mt19937 gen(rd());
		static std::uniform_int_distribution<> valDistrib(128, 255);
		static std::uniform_int_distribution<> satValDistrib(92, 255);
		
		// Используем индекс для создания вариаций saturation
		// Распределяем saturation равномерно по диапазону [192, 255]
		// Используем модуль для циклического распределения
		const int saturationRange = 64; // 255 - 192 + 1
		int saturation = 192 + (saturationIndex % saturationRange);
		saturation = satValDistrib(gen);
		// Генерируем случайное значение для value
		int value = valDistrib(gen);
		
		m_color = QColor::fromHsv(hue, saturation, value, 255);
	}
	else
	{
		// Для обычных параметров генерируем полностью случайный цвет
		static std::random_device rd;
		static std::mt19937 gen(rd());
		static std::uniform_int_distribution<> hueDistrib(0, 359);
		static std::uniform_int_distribution<> satValDistrib(192, 255);
		
		int hue = hueDistrib(gen);
		int saturation = satValDistrib(gen);
		int value = satValDistrib(gen);
		
		m_color = QColor::fromHsv(hue, saturation, value, 255);
	}
}

void ParameterTreeHistoryItem::addValue(const QVariant &value, const QDateTime &timestamp)
{
	QDateTime ts = timestamp;
	// Монотонность по X без «залипания» на одном ключе: иначе график
	// горизонталит, пока не появится новый timestamp
	if (!m_timestamps.isEmpty() && ts.isValid() && ts <= m_timestamps.last())
	{
		ts = m_timestamps.last().addMSecs(1);
	}
	m_values.append(value);
	m_timestamps.append(ts);
}

void ParameterTreeHistoryItem::setValues(const QList<QVariant>& values, const QList<QDateTime>& timestamps)
{
	m_values = values;
	m_timestamps = timestamps;
}

QVariant ParameterTreeHistoryItem::lastValue() const
{
	return m_values.last();
}

QVariant ParameterTreeHistoryItem::valueAtOrBefore(const QDateTime& time) const
{
	if (!time.isValid() || m_timestamps.isEmpty())
	{
		return QVariant();
	}

	// Вперёд за последнюю точку этой серии — данных ещё нет
	if (time > m_timestamps.last())
	{
		return QVariant();
	}

	const auto it = std::upper_bound(m_timestamps.cbegin(), m_timestamps.cend(), time);
	if (it == m_timestamps.cbegin())
	{
		return QVariant();
	}

	const int index = static_cast<int>(std::distance(m_timestamps.cbegin(), it)) - 1;
	return m_values.at(index);
}

const QList<QVariant>& ParameterTreeHistoryItem::values() const
{
	return m_values;
}

const QList<QDateTime>& ParameterTreeHistoryItem::timestamps() const
{
	return m_timestamps;
}

QDateTime ParameterTreeHistoryItem::lastTimestamp() const
{
	if (m_timestamps.isEmpty())
	{
		return QDateTime();
	}

	return m_timestamps.last();
}

QString ParameterTreeHistoryItem::control() const
{
	return m_control;
}

void ParameterTreeHistoryItem::setControl(const QString &control)
{
	m_control = control;
}

QVariant ParameterTreeHistoryItem::min() const
{
	return m_min;
}

void ParameterTreeHistoryItem::setMin(const QVariant &min)
{
	m_min = min;
}

QVariant ParameterTreeHistoryItem::max() const
{
	return m_max;
}

void ParameterTreeHistoryItem::setMax(const QVariant &max)
{
	m_max = max;
}