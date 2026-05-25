#include "ParameterTreeArrayItem.h"
#include <random>
#include <QColor>

ParameterTreeArrayItem::ParameterTreeArrayItem(const QString &label, ParameterTreeItem *parent)
    : ParameterTreeItem(label, parent)
{
	// Генерируем случайный hue для массива
	static std::random_device rd;
	static std::mt19937 gen(rd());
	static std::uniform_int_distribution<> hueDistrib(0, 359);
	
	m_arrayHue = hueDistrib(gen);
}

