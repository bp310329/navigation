#pragma once

#include <QGeoCoordinate>
#include <QList>
#include <QString>

// One step of the route, as returned by Valhalla
struct Maneuver {
    int type = 0;           // Valhalla maneuver type (e.g. 10 = right, 15 = left)
    QString instruction;    // "Turn right onto Mikołowska."
    int beginShapeIndex = 0;
    int endShapeIndex = 0;
    double length = 0.0;    // meters
    double time = 0.0;      // seconds
};

// Calculated route: geometry plus the maneuvers along it.
// Maneuver shape indices refer to `shape`.
struct Route {
    QList<QGeoCoordinate> shape;
    QList<Maneuver> maneuvers;
    double length = 0.0;    // meters
    double time = 0.0;      // seconds

    bool isValid() const { return shape.size() > 1 && !maneuvers.isEmpty(); }
};
