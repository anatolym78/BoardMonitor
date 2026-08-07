#include "ParametersChartView.h"
#include <QEvent>
#include <QMouseEvent>

namespace
{

// QCustomPlot рисует один фон на весь виджет, поэтому полупрозрачные подсветки
// QtCharts (view + chart area) заранее смешаны с белым.
const QColor kNormalBackground = QColor(Qt::white);
const QColor kSelectedBackground = QColor(227, 239, 255);
const QColor kHoveredBackground = QColor(255, 239, 193);

} // namespace

ParametersChartView::ParametersChartView(int chartIndex, int row, int column, QWidget* parent)
	: QCustomPlot(parent)
	, m_row(row)
	, m_column(column)
	, m_chartIndex(chartIndex)
	, m_selected(false)
	, m_hovered(false)
{
	setBackground(QBrush(kNormalBackground));
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
	QCustomPlot::enterEvent(event);
	
	m_hovered = true;

	updateBackground();
}

void ParametersChartView::leaveEvent(QEvent* event)
{
	QCustomPlot::leaveEvent(event);
	
	m_hovered = false;

	setHoveredGraph(nullptr);

	updateBackground();
}

void ParametersChartView::mousePressEvent(QMouseEvent* event)
{
	QCustomPlot::mousePressEvent(event);

	if (event->button() == Qt::MouseButton::LeftButton)
	{
		m_selected = !m_selected;

		m_chartsModel->selectElement(m_chartIndex, true);

		updateBackground();
	}
}

void ParametersChartView::mouseMoveEvent(QMouseEvent* event)
{
	QCustomPlot::mouseMoveEvent(event);

	setHoveredGraph(qobject_cast<QCPGraph*>(plottableAt(event->pos())));
}

void ParametersChartView::setHoveredGraph(QCPGraph* graph)
{
	if (m_hoveredGraph == graph)
	{
		return;
	}

	if (m_hoveredGraph)
	{
		emit graphHovered(m_hoveredGraph, false);
	}

	m_hoveredGraph = graph;

	if (m_hoveredGraph)
	{
		emit graphHovered(m_hoveredGraph, true);
	}
}

void ParametersChartView::updateBackground()
{
	if (m_hovered)
	{
		setBackground(QBrush(kHoveredBackground));
	}
	else if (m_selected)
	{
		setBackground(QBrush(kSelectedBackground));
	}
	else
	{
		setBackground(QBrush(kNormalBackground));
	}

	replot(QCustomPlot::rpQueued);
}
