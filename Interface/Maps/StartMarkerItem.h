#ifndef STARTMARKERITEM_H
#define STARTMARKERITEM_H

#include <QColor>
#include <QRectF>

#include <QGeoView/QGVDrawItem.h>
#include <QGeoView/QGVGlobal.h>

/**
 * @brief Полупрозрачный квадрат + крестик в стартовой точке траектории.
 * Размер квадрата — в метрах проекции (EPSG3857).
 */
class StartMarkerItem : public QGVDrawItem
{
	Q_OBJECT
public:
	explicit StartMarkerItem(QObject* parent = nullptr);

	void setCenter(const QGV::GeoPos& pos);
	QGV::GeoPos center() const { return m_geoCenter; }

	void setSquareSideMeters(double sideMeters);
	void setColor(const QColor& color);

protected:
	void onProjection(QGVMap* geoMap) override;
	QPainterPath projShape() const override;
	void projPaint(QPainter* painter) override;

private:
	void rebuildProjected();

	QGV::GeoPos m_geoCenter;
	QPointF m_projCenter;
	QRectF m_projSquare;
	double m_sideMeters = 200.0;
	QColor m_color{220, 40, 40};
	bool m_hasCenter = false;
};

#endif // STARTMARKERITEM_H
