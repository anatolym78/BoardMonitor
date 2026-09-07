#include "TrajectoryMapView.h"

#include <QDir>
#include <QNetworkAccessManager>
#include <QNetworkDiskCache>
#include <QTimer>
#include <QVBoxLayout>

#include <QGeoView/QGVCamera.h>
#include <QGeoView/QGVGlobal.h>
#include <QGeoView/QGVLayerGoogle.h>
#include <QGeoView/QGVMap.h>
#include <QGeoView/QGVWidgetCompass.h>
#include <QGeoView/QGVWidgetScale.h>
#include <QGeoView/QGVWidgetZoom.h>

namespace
{
constexpr double kMoscowLat = 55.7506;
constexpr double kMoscowLon = 37.6175;
/** Половина окна обзора (~несколько км вокруг центра). */
constexpr double kViewHalfDeg = 0.04;
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

	// Камера после первого layout — иначе scaleTo считает по нулевому viewport
	QTimer::singleShot(0, this, [this]() { centerOnMoscow(); });
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
	if (!m_map)
	{
		return;
	}

	const QGV::GeoPos moscow(kMoscowLat, kMoscowLon);
	const QGV::GeoRect area(
		kMoscowLat + kViewHalfDeg,
		kMoscowLon - kViewHalfDeg,
		kMoscowLat - kViewHalfDeg,
		kMoscowLon + kViewHalfDeg);

	m_map->cameraTo(QGVCameraActions(m_map).scaleTo(area).moveTo(moscow));
}
