#include "controller/NavigationController.h"

#include <cmath>

#include "location/LocationProvider.h"
#include "routing/RoutingService.h"

namespace {
// Closer than this to a stop counts as reached
constexpr double ArrivalDistance = 25.0;
// The "Then" maneuver is shown only when it follows the next one within this distance
constexpr double ThenManeuverDistance = 400.0;

// Farther than this from the route counts as off route...
constexpr double OffRouteDistance = 40.0;
// ...and staying off route this long triggers rerouting
constexpr qint64 OffRouteDelayMs = 3000;
// Minimum time between reroute requests (e.g. when far from any road)
constexpr qint64 RerouteCooldownMs = 10000;
}

NavigationController::NavigationController(LocationProvider *locationProvider,
                                           RoutingService *routingService,
                                           QObject *parent)
    : QObject(parent),
      m_locationProvider(locationProvider),
      m_routingService(routingService) {
    connect(m_locationProvider, &LocationProvider::positionUpdated,
            this, &NavigationController::onPositionUpdated);
    connect(m_routingService, &RoutingService::routeChanged,
            this, &NavigationController::onRouteChanged);
    // Request finished, whether with a new route or an error
    connect(m_routingService, &RoutingService::busyChanged, this, [this] {
        if (!m_routingService->busy())
            setRerouting(false);
    });
}

bool NavigationController::navigating() const { return m_navigating; }
bool NavigationController::arrived() const { return m_arrived; }
bool NavigationController::rerouting() const { return m_rerouting; }
QString NavigationController::maneuverType() const { return m_maneuverType; }
QString NavigationController::instruction() const { return m_instruction; }
double NavigationController::distanceToManeuver() const { return m_distanceToManeuver; }
QString NavigationController::thenManeuverType() const { return m_thenManeuverType; }
double NavigationController::remainingDistance() const { return m_remainingDistance; }
int NavigationController::remainingSeconds() const { return m_remainingSeconds; }
QVariantList NavigationController::stops() const { return m_stopsList; }
int NavigationController::stopCount() const { return int(m_tripStops.size()); }
int NavigationController::nextStopIndex() const { return m_reachedStops; }
QString NavigationController::nextStopName() const {
    return m_reachedStops < m_tripStops.size() ? m_tripStops.at(m_reachedStops).name : QString();
}
double NavigationController::distanceToNextStop() const { return m_distanceToNextStop; }
int NavigationController::secondsToNextStop() const { return m_secondsToNextStop; }
double NavigationController::heading() const { return m_heading; }
QVariantList NavigationController::traveledPath() const { return m_traveledPath; }
QVariantList NavigationController::remainingPath() const { return m_remainingPath; }

void NavigationController::startNavigation() {
    const Route &route = m_routingService->route();
    if (!route.isValid())
        return;

    startTrip(route.stops());
    m_tracker.setRoute(route);
    m_offRouteTimer.invalidate();
    m_lastReroute.invalidate();
    setRerouting(false);
    // Fill in the guidance before the panels become visible
    updateProgress(m_locationProvider->position());
    setNavigating(true);
}

void NavigationController::stopNavigation() {
    setNavigating(false);
    setRerouting(false);
    startTrip({});
}

void NavigationController::onPositionUpdated(const Position &position) {
    if (m_navigating)
        updateProgress(position);
}

void NavigationController::onRouteChanged() {
    if (!m_navigating)
        return;

    // A cleared route ends guidance; a new one (e.g. after rerouting) replaces it
    const Route &route = m_routingService->route();
    if (!route.isValid()) {
        stopNavigation();
        return;
    }

    // A route through the last stops of the trip continues it (rerouting);
    // any other route, e.g. a new one from the phone, starts a new trip
    const QList<Destination> routeStops = route.stops();
    const qsizetype offset = m_tripStops.size() - routeStops.size();
    if (offset >= 0 && m_tripStops.mid(offset) == routeStops) {
        m_routeFirstStop = int(offset);
    } else {
        startTrip(routeStops);
    }

    m_tracker.setRoute(route);
    m_offRouteTimer.invalidate();
    updateProgress(m_locationProvider->position());
}

void NavigationController::startTrip(const QList<Destination> &stops) {
    m_tripStops = stops;
    m_routeFirstStop = 0;
    m_reachedStops = 0;
    m_arrived = false;
    m_stopsList = RoutingService::stopsToVariantList(m_tripStops);
    emit stopsChanged();
}

void NavigationController::setReachedStops(int count) {
    count = qMin(count, int(m_tripStops.size()));
    if (count <= m_reachedStops)
        return;
    const int previous = m_reachedStops;
    m_reachedStops = count;
    m_stopsList = RoutingService::stopsToVariantList(m_tripStops, m_reachedStops);
    emit stopsChanged();
    for (int i = previous; i < count; ++i)
        emit stopReached(i);
}

void NavigationController::updateProgress(const Position &position) {
    if (!position.valid)
        return;

    const RouteTracker::Progress progress = m_tracker.update(position.coordinate());
    const Route &route = m_tracker.route();
    if (progress.nextManeuver < 0)
        return;

    // Stops are reached in order: those of the legs already driven, plus the
    // current one once close to it
    int reachedInRoute = progress.leg;
    if (m_tracker.distanceToLegEnd(progress, progress.leg) < ArrivalDistance)
        ++reachedInRoute;
    setReachedStops(m_routeFirstStop + reachedInRoute);
    m_arrived = m_reachedStops >= m_tripStops.size();

    const Maneuver &next = route.maneuvers.at(progress.nextManeuver);
    if (m_arrived) {
        m_maneuverType = QStringLiteral("arrive");
        m_instruction = instructionFor(int(route.maneuvers.size()) - 1);
        m_distanceToManeuver = progress.remainingDistance;
        m_thenManeuverType.clear();
    } else {
        m_maneuverType = maneuverKind(next.type);
        m_instruction = instructionFor(progress.nextManeuver);
        m_distanceToManeuver = progress.distanceToManeuver;

        // No "Then" after arriving at a stop: the next leg starts from there
        m_thenManeuverType.clear();
        const int thenIndex = progress.nextManeuver + 1;
        if (thenIndex < route.maneuvers.size() && legEndingWith(progress.nextManeuver) < 0) {
            const Maneuver &then = route.maneuvers.at(thenIndex);
            const double gap = m_tracker.distanceAt(then.beginShapeIndex)
                             - m_tracker.distanceAt(next.beginShapeIndex);
            if (gap < ThenManeuverDistance)
                m_thenManeuverType = maneuverKind(then.type);
        }
    }

    m_remainingDistance = progress.remainingDistance;
    m_remainingSeconds = int(std::lround(progress.remainingTime));

    const int nextStopLeg = m_reachedStops - m_routeFirstStop;
    m_distanceToNextStop = m_arrived ? 0.0 : m_tracker.distanceToLegEnd(progress, nextStopLeg);
    m_secondsToNextStop = m_arrived
        ? 0 : int(std::lround(m_tracker.timeToLegEnd(progress, nextStopLeg)));

    // No heading source yet: until the GPS reports a direction of travel,
    // the direction of the route at the current position stands in for it
    m_heading = position.hasHeading ? position.heading : progress.routeBearing;

    // Split the route at the snapped position into traveled and remaining parts
    m_traveledPath.clear();
    m_remainingPath.clear();
    for (int i = 0; i <= progress.segment; ++i)
        m_traveledPath.append(QVariant::fromValue(route.shape.at(i)));
    m_traveledPath.append(QVariant::fromValue(progress.snappedCoordinate));
    m_remainingPath.append(QVariant::fromValue(progress.snappedCoordinate));
    for (int i = progress.segment + 1; i < route.shape.size(); ++i)
        m_remainingPath.append(QVariant::fromValue(route.shape.at(i)));

    emit progressChanged();

    checkOffRoute(position, progress);
}

// Index of the leg whose "arrive" maneuver this is, or -1
int NavigationController::legEndingWith(int maneuverIndex) const {
    const QList<Leg> &legs = m_tracker.route().legs;
    for (int i = 0; i < legs.size(); ++i) {
        if (legs.at(i).lastManeuver == maneuverIndex)
            return i;
    }
    return -1;
}

// Valhalla's instruction, except for arriving at a stop: Valhalla does not
// know the stop names and calls every stop a destination
QString NavigationController::instructionFor(int maneuverIndex) const {
    const Route &route = m_tracker.route();
    const QString valhallaInstruction = route.maneuvers.at(maneuverIndex).instruction;

    const int leg = legEndingWith(maneuverIndex);
    if (leg < 0)
        return valhallaInstruction;

    const int stopIndex = m_routeFirstStop + leg;
    const bool finalStop = stopIndex == m_tripStops.size() - 1;
    const QString name = route.legs.at(leg).stop.name;
    if (!name.isEmpty())
        return tr("Arrive at %1").arg(name);
    if (!finalStop)
        return tr("Arrive at stop %1").arg(stopIndex + 1);
    return valhallaInstruction;
}

// Requests a new route from the current position once it has been away from
// the route for a while
void NavigationController::checkOffRoute(const Position &position,
                                         const RouteTracker::Progress &progress) {
    if (m_arrived || progress.distanceFromRoute <= OffRouteDistance) {
        m_offRouteTimer.invalidate();
        return;
    }

    if (!m_offRouteTimer.isValid()) {
        m_offRouteTimer.start();
        return;
    }
    if (m_offRouteTimer.elapsed() < OffRouteDelayMs)
        return;
    if (m_rerouting || m_routingService->busy())
        return;
    if (m_lastReroute.isValid() && m_lastReroute.elapsed() < RerouteCooldownMs)
        return;

    // Only through the stops not reached yet
    const QList<Destination> remainingStops = m_tripStops.mid(m_reachedStops);
    if (remainingStops.isEmpty())
        return;

    m_lastReroute.start();
    setRerouting(true);
    m_routingService->calculateRoute(position.coordinate(), remainingStops,
                                     position.hasHeading ? position.heading : -1.0);
}

void NavigationController::setRerouting(bool rerouting) {
    if (m_rerouting == rerouting)
        return;
    m_rerouting = rerouting;
    emit reroutingChanged();
}

void NavigationController::setNavigating(bool navigating) {
    if (m_navigating == navigating)
        return;
    m_navigating = navigating;
    emit navigatingChanged(m_navigating);
}

// Groups Valhalla maneuver types into the arrow kinds the UI can draw
QString NavigationController::maneuverKind(int valhallaType) {
    switch (valhallaType) {
    case 1: case 2: case 3:                     // start
        return QStringLiteral("depart");
    case 4: case 5: case 6:                     // destination
        return QStringLiteral("arrive");
    case 9: case 18: case 20: case 23: case 37: // slight right, ramp/exit/stay/merge right
        return QStringLiteral("slight-right");
    case 10:
        return QStringLiteral("right");
    case 11:
        return QStringLiteral("sharp-right");
    case 12:
        return QStringLiteral("uturn-right");
    case 13:
        return QStringLiteral("uturn-left");
    case 14:
        return QStringLiteral("sharp-left");
    case 15:
        return QStringLiteral("left");
    case 16: case 19: case 21: case 24: case 38: // slight left, ramp/exit/stay/merge left
        return QStringLiteral("slight-left");
    case 26: case 27:                           // roundabout enter/exit
        return QStringLiteral("roundabout");
    default:                                    // continue, becomes, stay straight, merge, ...
        return QStringLiteral("straight");
    }
}
