#pragma once

#include <QGeoCoordinate>
#include <QList>

#include "routing/Route.h"

// Follows the device along a route: snaps each position onto the route line
// and derives progress (distance/time left, next maneuver, current leg) from it.
// Plain logic with no Qt object or network dependencies.
class RouteTracker {
public:
    struct Progress {
        int segment = 0;                // route.shape[segment] -> [segment + 1]
        QGeoCoordinate snappedCoordinate;
        double distanceFromRoute = 0.0; // meters
        double traveledDistance = 0.0;  // meters along the route
        double remainingDistance = 0.0; // meters
        double remainingTime = 0.0;     // seconds
        int nextManeuver = -1;          // index into route.maneuvers
        double distanceToManeuver = 0.0;// meters
        double routeBearing = 0.0;      // degrees, direction of the route here
        int leg = 0;                    // index into route.legs: the leg being driven
    };

    void setRoute(const Route &route);
    const Route &route() const;

    // Distance along the route from its start to shape point `index`
    double distanceAt(int index) const;

    Progress update(const QGeoCoordinate &position);

    // Distance and expected time from the position in `progress` to the stop
    // at the end of `leg`; 0 for legs already passed
    double distanceToLegEnd(const Progress &progress, int leg) const;
    double timeToLegEnd(const Progress &progress, int leg) const;

private:
    struct Projection {
        double t = 0.0;         // 0..1 along the segment
        double distance = 0.0;  // meters from the position to the segment
    };

    Projection project(const QGeoCoordinate &position, int segment) const;
    int findSegment(const QGeoCoordinate &position) const;
    double bearingFrom(int segment) const;
    double timeToManeuver(const Progress &progress, int maneuver) const;

    Route m_route;
    QList<double> m_cumulative; // distance from the start to each shape point
    int m_segment = 0;          // last matched segment, the search starts here
};
