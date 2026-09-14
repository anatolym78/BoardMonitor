#include "GeoPolylineItem.h"

#include <QPainter>
#include <QPen>

#include <QGeoView/QGVMap.h>
#include <QGeoView/QGVProjection.h>

GeoPolylineItem::GeoPolylineItem(QObject* parent)
	: QGVDrawItem()
{
	Q_UNUSED(parent);
}

void GeoPolylineItem::clear()
{
	m_geoPoints.clear();
	m_projPoints.clear();
	resetBoundary();
	refresh();
}

void GeoPolylineItem::setPoints(const QList<QGV::GeoPos>& points)
{
	m_geoPoints = points;
	rebuildProjected();
	resetBoundary();
	refresh();
}

void GeoPolylineItem::appendPoint(const QGV::GeoPos& point)
{
	m_geoPoints.append(point);
	if (getMap() && getMap()->getProjection())
	{
		m_projPoints << getMap()->getProjection()->geoToProj(point);
	}
	resetBoundary();
	refresh();
}

void GeoPolylineItem::setColor(const QColor& color)
{
	m_color = color;
	repaint();
}

void GeoPolylineItem::onProjection(QGVMap* geoMap)
{
	QGVDrawItem::onProjection(geoMap);
	rebuildProjected();
}

void GeoPolylineItem::rebuildProjected()
{
	m_projPoints.clear();
	if (!getMap() || !getMap()->getProjection())
	{
		return;
	}
	auto* projection = getMap()->getProjection();
	for (const QGV::GeoPos& pos : m_geoPoints)
	{
		m_projPoints << projection->geoToProj(pos);
	}
}

QPainterPath GeoPolylineItem::projShape() const
{
	QPainterPath path;
	if (m_projPoints.isEmpty())
	{
		return path;
	}
	if (m_projPoints.size() == 1)
	{
		path.addEllipse(m_projPoints.first(), 4.0, 4.0);
		return path;
	}
	path.addPolygon(m_projPoints);
	return path;
}

void GeoPolylineItem::projPaint(QPainter* painter)
{
	if (m_projPoints.isEmpty())
	{
		return;
	}

	QPen pen(m_color, 3.0);
	pen.setCosmetic(true);
	pen.setJoinStyle(Qt::RoundJoin);
	pen.setCapStyle(Qt::RoundCap);
	painter->setPen(pen);
	painter->setBrush(Qt::NoBrush);

	if (m_projPoints.size() == 1)
	{
		painter->setBrush(m_color);
		painter->drawEllipse(m_projPoints.first(), 4.0, 4.0);
		return;
	}

	painter->drawPolyline(m_projPoints);
}
