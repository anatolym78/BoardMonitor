#ifndef TRAJECTORYMAPVIEW_H
#define TRAJECTORYMAPVIEW_H

#include <QDateTime>
#include <QPointer>
#include <QWidget>

#include <QGeoView/QGVGlobal.h>

class QGVMap;
class QGVLayer;
class GeoPolylineItem;
class StartMarkerItem;
class ParameterTreeStorage;
class ParameterTreeHistoryItem;

/**
 * @brief Вкладка «Траектория»: карта + красная полилиния lat/lon (прореживание ≥ 0.5 с).
 */
class TrajectoryMapView : public QWidget
{
	Q_OBJECT
public:
	explicit TrajectoryMapView(QWidget* parent = nullptr);

	QGVMap* map() const { return m_map; }

	void setStorage(ParameterTreeStorage* storage);
	void clearTrajectory();

private:
	void setupNetworkAccess();
	void centerOnMoscow();
	/** Центр в pos; ширина видимой области карты ≈ widthMeters. */
	void centerWithWidthMeters(const QGV::GeoPos& pos, double widthMeters);

	void onValueAdded(ParameterTreeHistoryItem* item);
	void tryResolveGpsItems();
	bool readLatestGps(double& latDeg, double& lonDeg) const;
	static bool toDegrees(const QVariant& raw, double& outDegrees);
	void considerAppendPoint();
	void rebuildFromHistory();
	void onFirstPoint(const QGV::GeoPos& pos);
	void appendTrackPoint(const QGV::GeoPos& pos);
	double projectedDistanceMeters(const QGV::GeoPos& a, const QGV::GeoPos& b) const;
	void updateStartMarkerVisibility();

	QGVMap* m_map = nullptr;
	QGVLayer* m_trackLayer = nullptr;
	GeoPolylineItem* m_polyline = nullptr;
	StartMarkerItem* m_startMarker = nullptr;

	QPointer<ParameterTreeStorage> m_storage;
	QMetaObject::Connection m_valueConnection;

	QPointer<ParameterTreeHistoryItem> m_latItem;
	QPointer<ParameterTreeHistoryItem> m_lonItem;

	QDateTime m_lastAppendTime;
	bool m_centeredOnTrack = false;
	double m_trackLengthMeters = 0.0;

	static constexpr int kMinIntervalMs = 500;
	static constexpr double kInitialViewWidthMeters = 5000.0;
	static constexpr double kHideFrameAfterMeters = 500.0;
	static constexpr double kStartSquareSideMeters = 200.0;
};

#endif // TRAJECTORYMAPVIEW_H
