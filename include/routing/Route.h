#pragma once

#include <QGeoCoordinate>
#include <QList>
#include <QString>

#include "data/Destination.h"

// One step of the route, as returned by Valhalla
struct Maneuver {
    int type = 0;           // Valhalla maneuver type (e.g. 10 = right, 15 = left)
    QString instruction;    // "Turn right onto Mikołowska."
    int beginShapeIndex = 0;
    int endShapeIndex = 0;
    double length = 0.0;    // meters
    double time = 0.0;      // seconds
};

// Part of the route leading to one of its stops. A route to a single
// destination has one leg; a multi-stop route has one leg per stop.
struct Leg {
    Destination stop;
    int endShapeIndex = 0;  // shape index of the stop
    int lastManeuver = 0;   // index of the leg's "arrive" maneuver in Route::maneuvers
    double length = 0.0;    // meters
    double time = 0.0;      // seconds
};

// Calculated route: geometry plus the maneuvers along it, split into legs.
// Maneuver and leg shape indices refer to `shape`. Consecutive legs share
// the shape point of the stop between them.
struct Route {
    QList<QGeoCoordinate> shape;
    QList<Maneuver> maneuvers;
    QList<Leg> legs;
    double length = 0.0;    // meters
    double time = 0.0;      // seconds

    bool isValid() const {
        return shape.size() > 1 && !maneuvers.isEmpty() && !legs.isEmpty();
    }

    // The stops in driving order; the last one is the final destination
    QList<Destination> stops() const {
        QList<Destination> result;
        result.reserve(legs.size());
        for (const Leg &leg : legs)
            result.append(leg.stop);
        return result;
    }
};
