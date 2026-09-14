#include "StartMarkerItem.h"

#include <QPainter>
#include <QPen>

#include <QGeoView/QGVMap.h>
#include <QGeoView/QGVProjection.h>

StartMarkerItem::StartMarkerItem(QObject* parent)
	: QGVDrawItem()
{
	Q_UNUSED(parent);
}

void StartMarkerItem::setCenter(const QGV::GeoPos& pos)
{
	m_geoCenter = pos;
	m_hasCenter = true;
	rebuildProjected();
	resetBoundary();
	refresh();
}

void StartMarkerItem::setSquareSideMeters(double sideMeters)
{
	m_sideMeters = qMax(1.0, sideMeters);
	rebuildProjected();
	resetBoundary();
	refresh();
}

void StartMarkerItem::setColor(const QColor& color)
{
	m_color = color;
	repaint();
}

void StartMarkerItem::onProjection(QGVMap* geoMap)
{
	QGVDrawItem::onProjection(geoMap);
	rebuildProjected();
}

void StartMarkerItem::rebuildProjected()
{
	m_projSquare = QRectF();
	m_projCenter = QPointF();
	if (!m_hasCenter || !getMap() || !getMap()->getProjection())
	{
		return;
	}

	auto* projection = getMap()->getProjection();
	m_projCenter = projection->geoToProj(m_geoCenter);
	const double half = m_sideMeters * 0.5;
	m_projSquare = QRectF(
		m_projCenter.x() - half,
		m_projCenter.y() - half,
		m_sideMeters,
		m_sideMeters);
}

QPainterPath StartMarkerItem::projShape() const
{
	QPainterPath path;
	if (!m_projSquare.isValid())
	{
		return path;
	}
	path.addRect(m_projSquare);
	return path;
}

void StartMarkerItem::projPaint(QPainter* painter)
{
	if (!m_projSquare.isValid())
	{
		return;
	}

	QColor fill = m_color;
	fill.setAlpha(55);
	QPen border(m_color, 2.0);
	border.setCosmetic(true);

	painter->setPen(border);
	painter->setBrush(fill);
	painter->drawRect(m_projSquare);

	QPen cross(m_color, 2.0);
	cross.setCosmetic(true);
	painter->setPen(cross);
	const double arm = m_sideMeters * 0.35;
	painter->drawLine(
		QPointF(m_projCenter.x() - arm, m_projCenter.y()),
		QPointF(m_projCenter.x() + arm, m_projCenter.y()));
	painter->drawLine(
		QPointF(m_projCenter.x(), m_projCenter.y() - arm),
		QPointF(m_projCenter.x(), m_projCenter.y() + arm));
}
