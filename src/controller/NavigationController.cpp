#include "controller/NavigationController.h"

#include <cmath>

#include "location/LocationProvider.h"
#include "routing/RoutingService.h"

namespace {
// Closer than this to the end of the route counts as arrived
constexpr double ArrivalDistance = 25.0;
// The "Then" maneuver is shown only when it follows the next one within this distance
constexpr double ThenManeuverDistance = 400.0;
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
}

bool NavigationController::navigating() const { return m_navigating; }
bool NavigationController::arrived() const { return m_arrived; }
QString NavigationController::maneuverType() const { return m_maneuverType; }
QString NavigationController::instruction() const { return m_instruction; }
double NavigationController::distanceToManeuver() const { return m_distanceToManeuver; }
QString NavigationController::thenManeuverType() const { return m_thenManeuverType; }
double NavigationController::remainingDistance() const { return m_remainingDistance; }
int NavigationController::remainingSeconds() const { return m_remainingSeconds; }
double NavigationController::heading() const { return m_heading; }
QVariantList NavigationController::traveledPath() const { return m_traveledPath; }
QVariantList NavigationController::remainingPath() const { return m_remainingPath; }

void NavigationController::startNavigation() {
    const Route &route = m_routingService->route();
    if (!route.isValid())
        return;

    m_tracker.setRoute(route);
    m_arrived = false;
    // Fill in the guidance before the panels become visible
    updateProgress(m_locationProvider->position());
    setNavigating(true);
}

void NavigationController::stopNavigation() {
    setNavigating(false);
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
    m_tracker.setRoute(route);
    updateProgress(m_locationProvider->position());
}

void NavigationController::updateProgress(const Position &position) {
    if (!position.valid)
        return;

    const RouteTracker::Progress progress = m_tracker.update(position.coordinate());
    const Route &route = m_tracker.route();
    if (progress.nextManeuver < 0)
        return;

    const Maneuver &next = route.maneuvers.at(progress.nextManeuver);
    m_arrived = m_arrived || progress.remainingDistance < ArrivalDistance;

    if (m_arrived) {
        m_maneuverType = QStringLiteral("arrive");
        m_instruction = route.maneuvers.last().instruction;
        m_distanceToManeuver = progress.remainingDistance;
        m_thenManeuverType.clear();
    } else {
        m_maneuverType = maneuverKind(next.type);
        m_instruction = next.instruction;
        m_distanceToManeuver = progress.distanceToManeuver;

        m_thenManeuverType.clear();
        const int thenIndex = progress.nextManeuver + 1;
        if (thenIndex < route.maneuvers.size()) {
            const Maneuver &then = route.maneuvers.at(thenIndex);
            const double gap = m_tracker.distanceAt(then.beginShapeIndex)
                             - m_tracker.distanceAt(next.beginShapeIndex);
            if (gap < ThenManeuverDistance)
                m_thenManeuverType = maneuverKind(then.type);
        }
    }

    m_remainingDistance = progress.remainingDistance;
    m_remainingSeconds = int(std::lround(progress.remainingTime));

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
