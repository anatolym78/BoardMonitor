#include "ChartView.h"

#include <QEvent>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QtMath>

namespace
{

const QColor kNormalBackground = QColor(Qt::white);
const QColor kSelectedBackground = QColor(227, 239, 255);
const QColor kHoveredBackground = QColor(255, 239, 193);

} // namespace

ChartView::ChartView(int chartIndex, int row, int column, QWidget* parent)
	: QCustomPlot(parent)
	, m_row(row)
	, m_column(column)
	, m_chartIndex(chartIndex)
	, m_selected(false)
	, m_hovered(false)
{
	setBackground(QBrush(kNormalBackground));
	setFocusPolicy(Qt::WheelFocus);

	m_zoomLabel = new QCPItemText(this);
	m_zoomLabel->setPositionAlignment(Qt::AlignTop | Qt::AlignRight);
	m_zoomLabel->position->setType(QCPItemPosition::ptAxisRectRatio);
	m_zoomLabel->position->setAxisRect(axisRect());
	m_zoomLabel->position->setCoords(0.98, 0.04);
	m_zoomLabel->setTextAlignment(Qt::AlignRight);
	m_zoomLabel->setFont(QFont(font().family(), 9));
	m_zoomLabel->setColor(QColor(80, 80, 80));
	m_zoomLabel->setPadding(QMargins(4, 2, 4, 2));
	m_zoomLabel->setBrush(QBrush(QColor(255, 255, 255, 180)));
	m_zoomLabel->setPen(Qt::NoPen);
	m_zoomLabel->setClipToAxisRect(false);
	updateZoomLabel();
}

void ChartView::setSelected(bool selected)
{
	if (m_selected == selected)
	{
		return;
	}

	m_selected = selected;
	updateBackground();
}

void ChartView::setHovered(bool hover)
{
	m_hovered = hover;
	updateBackground();
}

void ChartView::setValueBaseRange(double lower, double upper)
{
	if (upper < lower)
	{
		qSwap(lower, upper);
	}
	m_baseLower = lower;
	m_baseUpper = upper;
	m_hasBaseRange = true;
	applyZoomedRange(nullptr);
}

void ChartView::setValueZoom(double zoom, double anchorY)
{
	m_valueZoom = qBound(kZoomMin, zoom, kZoomMax);
	applyZoomedRange(&anchorY);
	updateZoomLabel();
	replot(QCustomPlot::rpQueued);
}

void ChartView::resetValueZoom()
{
	if (qFuzzyCompare(m_valueZoom, 1.0) && m_hasBaseRange)
	{
		applyZoomedRange(nullptr);
		updateZoomLabel();
		replot(QCustomPlot::rpQueued);
		return;
	}
	m_valueZoom = 1.0;
	applyZoomedRange(nullptr);
	updateZoomLabel();
	replot(QCustomPlot::rpQueued);
}

void ChartView::applyZoomedRange(const double* anchorY)
{
	if (!yAxis || !m_hasBaseRange)
	{
		return;
	}

	double baseSpan = m_baseUpper - m_baseLower;
	if (baseSpan < 1e-12)
	{
		baseSpan = 1e-12;
	}
	const double visibleSpan = baseSpan / m_valueZoom;

	double fraction = 0.5;
	double anchor = (m_baseLower + m_baseUpper) * 0.5;

	if (anchorY)
	{
		anchor = *anchorY;
		const QCPRange old = yAxis->range();
		if (old.size() > 1e-12)
		{
			fraction = (anchor - old.lower) / old.size();
			fraction = qBound(0.0, fraction, 1.0);
		}
	}
	else if (!qFuzzyCompare(m_valueZoom, 1.0))
	{
		anchor = yAxis->range().center();
		fraction = 0.5;
	}

	const double lower = anchor - fraction * visibleSpan;
	yAxis->setRange(lower, lower + visibleSpan);
}

void ChartView::updateZoomLabel()
{
	if (!m_zoomLabel)
	{
		return;
	}
	m_zoomLabel->setText(QStringLiteral("×%1").arg(m_valueZoom, 0, 'f', 2));
	m_zoomLabel->setVisible(true);
}

void ChartView::enterEvent(QEvent* event)
{
	QCustomPlot::enterEvent(event);
	m_hovered = true;
	updateBackground();
}

void ChartView::leaveEvent(QEvent* event)
{
	QCustomPlot::leaveEvent(event);
	m_hovered = false;
	setHoveredGraph(nullptr);
	updateBackground();
}

void ChartView::mousePressEvent(QMouseEvent* event)
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
	const QPoint pos = event->position().toPoint();
#else
	const QPoint pos = event->pos();
#endif
	if (event->button() == Qt::LeftButton
		&& axisRect()
		&& axisRect()->rect().contains(pos))
	{
		resetValueZoom();
	}

	QCustomPlot::mousePressEvent(event);

	if (event->button() == Qt::MouseButton::LeftButton)
	{
		m_selected = !m_selected;
		if (m_chartsModel)
		{
			m_chartsModel->selectChart(m_chartIndex, true);
		}
		updateBackground();
	}
}

void ChartView::mouseMoveEvent(QMouseEvent* event)
{
	QCustomPlot::mouseMoveEvent(event);
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
	const QPoint pos = event->position().toPoint();
#else
	const QPoint pos = event->pos();
#endif
	setHoveredGraph(qobject_cast<QCPGraph*>(plottableAt(pos)));
}

void ChartView::wheelEvent(QWheelEvent* event)
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
	const QPoint pos = event->position().toPoint();
#else
	const QPoint pos = event->pos();
#endif
	if (!yAxis || !m_hasBaseRange || !axisRect()
		|| !axisRect()->rect().contains(pos))
	{
		event->ignore();
		return;
	}

	const QPoint delta = event->angleDelta();
	if (delta.isNull())
	{
		event->ignore();
		return;
	}

	const double steps = delta.y() / 120.0;
	const double factor = qPow(1.15, steps);
	const double anchorY = yAxis->pixelToCoord(pos.y());
	setValueZoom(m_valueZoom * factor, anchorY);
	event->accept();
}

void ChartView::setHoveredGraph(QCPGraph* graph)
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

void ChartView::updateBackground()
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
