#include "navigation/RouteTracker.h"

#include <cmath>
#include <limits>

namespace {
constexpr double MetersPerDegree = 111195.0; // on a 6371 km sphere
constexpr double DegreesToRadians = 3.14159265358979323846 / 180.0;

// Segments searched ahead of the last match. A forward window keeps the match
// from jumping to a later part of the route that passes nearby.
constexpr int SearchAhead = 200;
constexpr int SearchBehind = 2;
// Beyond this distance the window match is distrusted and the whole route is searched
constexpr double WindowMatchLimit = 50.0;
}

void RouteTracker::setRoute(const Route &route) {
    m_route = route;
    m_segment = 0;

    m_cumulative.clear();
    m_cumulative.reserve(route.shape.size());
    double total = 0.0;
    for (qsizetype i = 0; i < route.shape.size(); ++i) {
        if (i > 0)
            total += route.shape.at(i - 1).distanceTo(route.shape.at(i));
        m_cumulative.append(total);
    }
}

const Route &RouteTracker::route() const {
    return m_route;
}

double RouteTracker::distanceAt(int index) const {
    if (m_cumulative.isEmpty())
        return 0.0;
    return m_cumulative.at(qBound(0, index, int(m_cumulative.size()) - 1));
}

RouteTracker::Progress RouteTracker::update(const QGeoCoordinate &position) {
    Progress progress;
    if (!m_route.isValid() || !position.isValid())
        return progress;

    m_segment = findSegment(position);
    const Projection projection = project(position, m_segment);

    const QGeoCoordinate &a = m_route.shape.at(m_segment);
    const QGeoCoordinate &b = m_route.shape.at(m_segment + 1);
    const double segmentLength = distanceAt(m_segment + 1) - distanceAt(m_segment);
    const double totalLength = m_cumulative.last();

    progress.segment = m_segment;
    progress.snappedCoordinate = QGeoCoordinate(
        a.latitude() + projection.t * (b.latitude() - a.latitude()),
        a.longitude() + projection.t * (b.longitude() - a.longitude()));
    progress.distanceFromRoute = projection.distance;
    progress.traveledDistance = distanceAt(m_segment) + projection.t * segmentLength;
    progress.remainingDistance = qMax(0.0, totalLength - progress.traveledDistance);
    progress.routeBearing = bearingFrom(m_segment);

    // Next maneuver: the first one starting after the current segment.
    // The final "arrive" maneuver starts at the last shape point, so one is always found.
    const QList<Maneuver> &maneuvers = m_route.maneuvers;
    int next = maneuvers.size() - 1;
    for (int i = 0; i < maneuvers.size(); ++i) {
        if (maneuvers.at(i).beginShapeIndex > m_segment) {
            next = i;
            break;
        }
    }
    progress.nextManeuver = next;
    progress.distanceToManeuver = qMax(
        0.0, distanceAt(maneuvers.at(next).beginShapeIndex) - progress.traveledDistance);

    // Remaining time: the untraveled share of the current maneuver plus all later ones
    double remainingTime = 0.0;
    for (int i = next; i < maneuvers.size(); ++i)
        remainingTime += maneuvers.at(i).time;
    if (next > 0) {
        const Maneuver &current = maneuvers.at(next - 1);
        const double currentLength = distanceAt(maneuvers.at(next).beginShapeIndex)
                                   - distanceAt(current.beginShapeIndex);
        if (currentLength > 0.0)
            remainingTime += current.time * progress.distanceToManeuver / currentLength;
    }
    progress.remainingTime = remainingTime;

    return progress;
}

// Projects the position onto a segment in a local flat approximation, which is
// accurate enough over the length of a single route segment.
RouteTracker::Projection RouteTracker::project(const QGeoCoordinate &position,
                                               int segment) const {
    const QGeoCoordinate &a = m_route.shape.at(segment);
    const QGeoCoordinate &b = m_route.shape.at(segment + 1);
    const double lonScale = std::cos(position.latitude() * DegreesToRadians);

    // Meters relative to the position
    const double ax = (a.longitude() - position.longitude()) * lonScale * MetersPerDegree;
    const double ay = (a.latitude() - position.latitude()) * MetersPerDegree;
    const double bx = (b.longitude() - position.longitude()) * lonScale * MetersPerDegree;
    const double by = (b.latitude() - position.latitude()) * MetersPerDegree;

    const double dx = bx - ax;
    const double dy = by - ay;
    const double lengthSquared = dx * dx + dy * dy;

    Projection projection;
    projection.t = lengthSquared > 0.0
                 ? qBound(0.0, -(ax * dx + ay * dy) / lengthSquared, 1.0)
                 : 0.0;
    const double px = ax + projection.t * dx;
    const double py = ay + projection.t * dy;
    projection.distance = std::sqrt(px * px + py * py);
    return projection;
}

int RouteTracker::findSegment(const QGeoCoordinate &position) const {
    const int segmentCount = int(m_route.shape.size()) - 1;

    auto bestIn = [&](int first, int last, double &bestDistance) {
        int best = first;
        bestDistance = std::numeric_limits<double>::max();
        for (int i = first; i <= last; ++i) {
            const double distance = project(position, i).distance;
            if (distance < bestDistance) {
                bestDistance = distance;
                best = i;
            }
        }
        return best;
    };

    double distance = 0.0;
    const int best = bestIn(qMax(0, m_segment - SearchBehind),
                            qMin(segmentCount - 1, m_segment + SearchAhead),
                            distance);
    if (distance <= WindowMatchLimit)
        return best;

    double globalDistance = 0.0;
    const int globalBest = bestIn(0, segmentCount - 1, globalDistance);
    return globalDistance < distance ? globalBest : best;
}

// Direction of the route at a segment, skipping zero-length segments
double RouteTracker::bearingFrom(int segment) const {
    for (int i = segment; i + 1 < m_route.shape.size(); ++i) {
        const QGeoCoordinate &a = m_route.shape.at(i);
        const QGeoCoordinate &b = m_route.shape.at(i + 1);
        if (a.distanceTo(b) > 0.5)
            return a.azimuthTo(b);
    }
    return 0.0;
}
