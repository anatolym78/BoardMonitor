#ifndef GEOPOLYLINEITEM_H
#define GEOPOLYLINEITEM_H

#include <QColor>
#include <QList>
#include <QPolygonF>

#include <QGeoView/QGVDrawItem.h>
#include <QGeoView/QGVGlobal.h>

/**
 * @brief Ломаная на карте в географических координатах (lat/lon).
 */
class GeoPolylineItem : public QGVDrawItem
{
	Q_OBJECT
public:
	explicit GeoPolylineItem(QObject* parent = nullptr);

	void clear();
	void setPoints(const QList<QGV::GeoPos>& points);
	void appendPoint(const QGV::GeoPos& point);
	QList<QGV::GeoPos> points() const { return m_geoPoints; }
	int pointCount() const { return m_geoPoints.size(); }

	void setColor(const QColor& color);
	QColor color() const { return m_color; }

protected:
	void onProjection(QGVMap* geoMap) override;
	QPainterPath projShape() const override;
	void projPaint(QPainter* painter) override;

private:
	void rebuildProjected();

	QList<QGV::GeoPos> m_geoPoints;
	QPolygonF m_projPoints;
	QColor m_color{45, 125, 210};
};

#endif // GEOPOLYLINEITEM_H
