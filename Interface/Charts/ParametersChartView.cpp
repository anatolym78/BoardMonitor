#include "ParametersChartView.h"
#include <QtCharts/QChart>
#include <QEvent>

ParametersChartView::ParametersChartView(int chartIndex, int row, int column, QWidget* parent)
	: QtCharts::QChartView(parent)
	, m_selected(false)
	, m_hovered(false)
	, m_row(row)
	, m_column(column)
	, m_chartIndex(chartIndex)
{
	// Устанавливаем белый фон для view по умолчанию
	setBackgroundBrush(QBrush(Qt::white));
}

void ParametersChartView::setSelected(bool selected)
{
	if (m_selected == selected)
	{
		return;
	}
	
	m_selected = selected;
	updateBackground();
}

void ParametersChartView::setHovered(bool hover)
{
	m_hovered = hover;

	updateBackground();
}

void ParametersChartView::enterEvent(QEvent* event)
{
	QtCharts::QChartView::enterEvent(event);
	
	m_hovered = true;

	updateBackground();
}

void ParametersChartView::leaveEvent(QEvent* event)
{
	QtCharts::QChartView::leaveEvent(event);
	
	m_hovered = false;

	updateBackground();
}

void ParametersChartView::mousePressEvent(QMouseEvent* event)
{
	if (event->button() == Qt::MouseButton::LeftButton)
	{
		m_selected = !m_selected;

		m_chartsModel->selectElement(m_chartIndex, true);

		updateBackground();
	}
}

void ParametersChartView::updateBackground()
{
	// Обновляем фон view в зависимости от состояния selected
	if (m_selected)
	{
		// Полупрозрачный синий при выделении
		setBackgroundBrush(QBrush(QColor::fromHsv(215, 225, 255, 32)));
	}
	else
	{
		// Белый при снятии выделения
		setBackgroundBrush(QBrush(Qt::white));
	}

	if (m_hovered)
	{
		chart()->setBackgroundBrush(QBrush(QColor::fromHsv(45, 255, 255, 62)));
	}
	else
	{
		chart()->setBackgroundBrush(QBrush(QColor(0, 0, 0, 0)));
	}
}

