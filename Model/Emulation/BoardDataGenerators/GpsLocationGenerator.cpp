#include "GpsLocationGenerator.h"
#include "./../../Parameters/BoardParameterSingle.h"
#include <QVariantList>
#include <cmath>
#include <QDateTime>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

GpsLocationGenerator::GpsLocationGenerator(QObject *parent)
    : ParameterGenerator(parent)
{
}

QString GpsLocationGenerator::getName() const
{
    return "telemetry.gpsLocation";
}

QList<BoardParameterSingle*> GpsLocationGenerator::generate(double time)
{
    const double base_lat = 55.7558; // Moscow latitude
    const double base_lon = 37.6173; // Moscow longitude
    const double base_alt = 156.0;   // Moscow altitude

    double lat = base_lat + 0.01 * sin(2 * M_PI * time / 120.0);
    double lon = base_lon + 0.01 * cos(2 * M_PI * time / 120.0);
    double alt = base_alt + 10.0 * sin(2 * M_PI * time / 50.0);

    QVariantList gps_values;
    gps_values.append(lat);
    gps_values.append(lon);
    gps_values.append(alt);
    
    BoardParameterSingle* p = new BoardParameterSingle(getName(), gps_values, QDateTime::currentDateTime());
    return {p};
}
