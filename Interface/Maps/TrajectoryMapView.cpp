#include "TrajectoryMapView.h"
#include "GeoPolylineItem.h"
#include "StartMarkerItem.h"

#include "../../Model/Parameters/Tree/ParameterTreeHistoryItem.h"
#include "../../Model/Parameters/Tree/ParameterTreeItem.h"
#include "../../Model/Parameters/Tree/ParameterTreeStorage.h"

#include <QDir>
#include <QLineF>
#include <QNetworkAccessManager>
#include <QNetworkDiskCache>
#include <QTimer>
#include <QVBoxLayout>
#include <QtMath>

#include <QGeoView/QGVCamera.h>
#include <QGeoView/QGVGlobal.h>
#include <QGeoView/QGVLayer.h>
#include <QGeoView/QGVLayerGoogle.h>
#include <QGeoView/QGVMap.h>
#include <QGeoView/QGVMapQGView.h>
#include <QGeoView/QGVProjection.h>
#include <QGeoView/QGVWidgetCompass.h>
#include <QGeoView/QGVWidgetScale.h>
#include <QGeoView/QGVWidgetZoom.h>

namespace
{
constexpr double kMoscowLat = 55.7506;
constexpr double kMoscowLon = 37.6175;
constexpr double kMoscowViewWidthMeters = 5000.0;

bool labelIsLat(const QString& label)
{
	const QString l = label.toLower();
	return l == QLatin1String("lat")
		|| l == QLatin1String("latitude")
		|| l == QLatin1String("gps_lat");
}

bool labelIsLon(const QString& label)
{
	const QString l = label.toLower();
	return l == QLatin1String("lon")
		|| l == QLatin1String("lng")
		|| l == QLatin1String("longitude")
		|| l == QLatin1String("gps_lon");
}

bool parentLooksLikeGps(const ParameterTreeItem* item)
{
	if (!item || !item->parentItem())
	{
		return false;
	}
	const QString p = item->parentItem()->label().toLower();
	return p.contains(QLatin1String("gps"))
		|| p.contains(QLatin1String("coord"))
		|| p.contains(QLatin1String("location"));
}

void findLatLonRecursive(ParameterTreeItem* node,
	ParameterTreeHistoryItem*& latOut,
	ParameterTreeHistoryItem*& lonOut)
{
	if (!node || (latOut && lonOut))
	{
		return;
	}

	if (node->type() == ParameterTreeItem::ItemType::History)
	{
		auto* history = static_cast<ParameterTreeHistoryItem*>(node);
		const QString label = history->label();
		if (!latOut && labelIsLat(label))
		{
			latOut = history;
		}
		else if (!lonOut && labelIsLon(label))
		{
			lonOut = history;
		}
		else if (parentLooksLikeGps(history))
		{
			if (!latOut && label == QLatin1String("0"))
			{
				latOut = history;
			}
			else if (!lonOut && label == QLatin1String("1"))
			{
				lonOut = history;
			}
		}
	}

	for (ParameterTreeItem* child : node->children())
	{
		findLatLonRecursive(child, latOut, lonOut);
		if (latOut && lonOut)
		{
			return;
		}
	}
}
}

TrajectoryMapView::TrajectoryMapView(QWidget* parent)
	: QWidget(parent)
{
	auto* layout = new QVBoxLayout(this);
	layout->setContentsMargins(0, 0, 0, 0);
	layout->setSpacing(0);

	setupNetworkAccess();

	m_map = new QGVMap(this);
	layout->addWidget(m_map);

	m_map->addWidget(new QGVWidgetCompass());
	m_map->addWidget(new QGVWidgetZoom());
	m_map->addWidget(new QGVWidgetScale());

	auto* google = new QGVLayerGoogle(QGV::TilesType::Schema);
	m_map->addItem(google);

	m_trackLayer = new QGVLayer();
	m_map->addItem(m_trackLayer);

	m_polyline = new GeoPolylineItem();
	m_polyline->setColor(QColor(220, 40, 40));
	m_trackLayer->addItem(m_polyline);

	m_startMarker = new StartMarkerItem();
	m_startMarker->setColor(QColor(220, 40, 40));
	m_startMarker->setSquareSideMeters(kStartSquareSideMeters);
	m_startMarker->hide();
	m_trackLayer->addItem(m_startMarker);

	QTimer::singleShot(0, this, [this]() { centerOnMoscow(); });
}

void TrajectoryMapView::setStorage(ParameterTreeStorage* storage)
{
	if (m_valueConnection)
	{
		disconnect(m_valueConnection);
	}

	m_storage = storage;
	m_latItem.clear();
	m_lonItem.clear();
	clearTrajectory();

	if (!m_storage)
	{
		return;
	}

	tryResolveGpsItems();
	rebuildFromHistory();
	m_valueConnection = connect(m_storage, &ParameterTreeStorage::valueAdded,
		this, &TrajectoryMapView::onValueAdded);
}

void TrajectoryMapView::clearTrajectory()
{
	m_lastAppendTime = QDateTime();
	m_centeredOnTrack = false;
	m_trackLengthMeters = 0.0;
	if (m_polyline)
	{
		m_polyline->clear();
	}
	if (m_startMarker)
	{
		m_startMarker->hide();
	}
}

void TrajectoryMapView::setupNetworkAccess()
{
	if (QGV::getNetworkManager() != nullptr)
	{
		return;
	}

	auto* cache = new QNetworkDiskCache(this);
	cache->setCacheDirectory(QDir::temp().filePath(QStringLiteral("BoardMonitor-qgeoview-cache")));

	auto* manager = new QNetworkAccessManager(this);
	manager->setCache(cache);
	QGV::setNetworkManager(manager);
}

void TrajectoryMapView::centerOnMoscow()
{
	centerWithWidthMeters(QGV::GeoPos(kMoscowLat, kMoscowLon), kMoscowViewWidthMeters);
}

void TrajectoryMapView::centerWithWidthMeters(const QGV::GeoPos& pos, double widthMeters)
{
	if (!m_map || !m_map->getProjection())
	{
		return;
	}

	const QPointF center = m_map->getProjection()->geoToProj(pos);
	double aspect = 1.0;
	if (auto* view = m_map->geoView())
	{
		const QSize sz = view->size();
		if (sz.width() > 0 && sz.height() > 0)
		{
			aspect = double(sz.height()) / double(sz.width());
		}
	}

	const double heightMeters = widthMeters * aspect;
	const QRectF area(
		center.x() - widthMeters * 0.5,
		center.y() - heightMeters * 0.5,
		widthMeters,
		heightMeters);

	m_map->cameraTo(QGVCameraActions(m_map).scaleTo(area).moveTo(pos));
}

void TrajectoryMapView::onValueAdded(ParameterTreeHistoryItem* item)
{
	if (!item)
	{
		return;
	}

	if (!m_latItem || !m_lonItem)
	{
		tryResolveGpsItems();
	}

	if (item != m_latItem && item != m_lonItem)
	{
		return;
	}

	considerAppendPoint();
}

void TrajectoryMapView::tryResolveGpsItems()
{
	if (!m_storage)
	{
		return;
	}

	ParameterTreeHistoryItem* lat = nullptr;
	ParameterTreeHistoryItem* lon = nullptr;
	findLatLonRecursive(m_storage, lat, lon);
	m_latItem = lat;
	m_lonItem = lon;
}

bool TrajectoryMapView::toDegrees(const QVariant& raw, double& outDegrees)
{
	bool ok = false;
	const double v = raw.toDouble(&ok);
	if (!ok || !qIsFinite(v))
	{
		return false;
	}

	outDegrees = (qAbs(v) > 180.0) ? (v / 1e7) : v;
	return qIsFinite(outDegrees);
}

bool TrajectoryMapView::readLatestGps(double& latDeg, double& lonDeg) const
{
	if (!m_latItem || !m_lonItem
		|| m_latItem->values().isEmpty()
		|| m_lonItem->values().isEmpty())
	{
		return false;
	}

	if (!toDegrees(m_latItem->lastValue(), latDeg)
		|| !toDegrees(m_lonItem->lastValue(), lonDeg))
	{
		return false;
	}

	if (latDeg < -90.0 || latDeg > 90.0 || lonDeg < -180.0 || lonDeg > 180.0)
	{
		return false;
	}

	return true;
}

double TrajectoryMapView::projectedDistanceMeters(const QGV::GeoPos& a, const QGV::GeoPos& b) const
{
	if (!m_map || !m_map->getProjection())
	{
		return 0.0;
	}
	const QPointF pa = m_map->getProjection()->geoToProj(a);
	const QPointF pb = m_map->getProjection()->geoToProj(b);
	return QLineF(pa, pb).length();
}

void TrajectoryMapView::updateStartMarkerVisibility()
{
	if (!m_startMarker)
	{
		return;
	}
	m_startMarker->setVisible(m_trackLengthMeters < kHideFrameAfterMeters);
}

void TrajectoryMapView::onFirstPoint(const QGV::GeoPos& pos)
{
	m_centeredOnTrack = true;
	centerWithWidthMeters(pos, kInitialViewWidthMeters);

	if (m_startMarker)
	{
		m_startMarker->setCenter(pos);
		m_startMarker->show();
	}
	updateStartMarkerVisibility();
}

void TrajectoryMapView::appendTrackPoint(const QGV::GeoPos& pos)
{
	if (!m_polyline)
	{
		return;
	}

	if (m_polyline->pointCount() > 0)
	{
		m_trackLengthMeters += projectedDistanceMeters(m_polyline->points().constLast(), pos);
	}

	m_polyline->appendPoint(pos);
	updateStartMarkerVisibility();
}

void TrajectoryMapView::considerAppendPoint()
{
	double lat = 0.0;
	double lon = 0.0;
	if (!readLatestGps(lat, lon))
	{
		return;
	}

	const QDateTime now = QDateTime::currentDateTime();
	if (m_lastAppendTime.isValid()
		&& m_lastAppendTime.msecsTo(now) < kMinIntervalMs)
	{
		return;
	}

	const QGV::GeoPos pos(lat, lon);
	if (m_polyline && m_polyline->pointCount() > 0)
	{
		const QGV::GeoPos last = m_polyline->points().constLast();
		if (qFuzzyCompare(last.latitude() + 1.0, pos.latitude() + 1.0)
			&& qFuzzyCompare(last.longitude() + 1.0, pos.longitude() + 1.0))
		{
			return;
		}
	}

	m_lastAppendTime = now;

	const bool first = !m_centeredOnTrack;
	appendTrackPoint(pos);
	if (first)
	{
		onFirstPoint(pos);
	}
}

void TrajectoryMapView::rebuildFromHistory()
{
	if (!m_polyline || !m_latItem || !m_lonItem)
	{
		return;
	}

	const auto& latTimes = m_latItem->timestamps();
	const auto& latValues = m_latItem->values();
	const auto& lonTimes = m_lonItem->timestamps();
	const auto& lonValues = m_lonItem->values();
	if (latTimes.isEmpty() || lonTimes.isEmpty())
	{
		return;
	}

	QList<QGV::GeoPos> points;
	QDateTime lastKept;
	const int n = qMin(latTimes.size(), latValues.size());
	int lonIndex = 0;

	for (int i = 0; i < n; ++i)
	{
		const QDateTime& t = latTimes[i];
		while (lonIndex + 1 < lonTimes.size() && lonTimes[lonIndex + 1] <= t)
		{
			++lonIndex;
		}
		if (lonIndex >= lonValues.size())
		{
			break;
		}

		if (lastKept.isValid() && lastKept.msecsTo(t) < kMinIntervalMs)
		{
			continue;
		}

		double latDeg = 0.0;
		double lonDeg = 0.0;
		if (!toDegrees(latValues[i], latDeg) || !toDegrees(lonValues[lonIndex], lonDeg))
		{
			continue;
		}
		if (latDeg < -90.0 || latDeg > 90.0 || lonDeg < -180.0 || lonDeg > 180.0)
		{
			continue;
		}

		points.append(QGV::GeoPos(latDeg, lonDeg));
		lastKept = t;
	}

	if (points.isEmpty())
	{
		return;
	}

	m_trackLengthMeters = 0.0;
	for (int i = 1; i < points.size(); ++i)
	{
		m_trackLengthMeters += projectedDistanceMeters(points[i - 1], points[i]);
	}

	m_polyline->setPoints(points);
	m_lastAppendTime = lastKept.isValid() ? lastKept : QDateTime::currentDateTime();

	onFirstPoint(points.constFirst());
	updateStartMarkerVisibility();
}
