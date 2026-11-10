#pragma once

#include <QGeoCoordinate>
#include <QString>

// A place to drive to: a single destination, or one stop of a multi-stop route
struct Destination {
    QString name;
    double latitude = 0.0;
    double longitude = 0.0;

    QGeoCoordinate coordinate() const { return QGeoCoordinate(latitude, longitude); }

    bool operator==(const Destination &other) const {
        return name == other.name && latitude == other.latitude
            && longitude == other.longitude;
    }
    bool operator!=(const Destination &other) const { return !(*this == other); }
};
