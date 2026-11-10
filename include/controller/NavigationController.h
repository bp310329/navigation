#pragma once

#include <QElapsedTimer>
#include <QObject>
#include <QVariantList>
#include <QtQml/qqmlregistration.h>

#include "data/Position.h"
#include "navigation/RouteTracker.h"

class LocationProvider;
class RoutingService;

// Turn-by-turn guidance along the route from RoutingService: follows the
// position from LocationProvider and exposes the next maneuver, the remaining
// distance and time, and the route split into traveled and remaining parts.
//
// A route may have several stops (a trip). They are reached in order; after
// leaving the route, the new route goes only through the stops not reached yet.
class NavigationController : public QObject {
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Provided by the application")

    Q_PROPERTY(bool navigating READ navigating NOTIFY navigatingChanged)
    // The final destination is reached
    Q_PROPERTY(bool arrived READ arrived NOTIFY progressChanged)
    // A new route is being calculated after leaving the current one
    Q_PROPERTY(bool rerouting READ rerouting NOTIFY reroutingChanged)

    // Next maneuver: kind for the icon ("left", "right", "slight-right",
    // "roundabout", "arrive", ...), instruction text and distance to it
    Q_PROPERTY(QString maneuverType READ maneuverType NOTIFY progressChanged)
    Q_PROPERTY(QString instruction READ instruction NOTIFY progressChanged)
    Q_PROPERTY(double distanceToManeuver READ distanceToManeuver NOTIFY progressChanged)
    // Kind of the maneuver after the next one when it follows shortly, else empty
    Q_PROPERTY(QString thenManeuverType READ thenManeuverType NOTIFY progressChanged)

    // To the final destination
    Q_PROPERTY(double remainingDistance READ remainingDistance NOTIFY progressChanged)
    Q_PROPERTY(int remainingSeconds READ remainingSeconds NOTIFY progressChanged)

    // Stops of the trip in driving order: [{ name, coordinate, reached }].
    // The last one is the final destination.
    Q_PROPERTY(QVariantList stops READ stops NOTIFY stopsChanged)
    Q_PROPERTY(int stopCount READ stopCount NOTIFY stopsChanged)
    // Index of the stop being driven to; equals stopCount once arrived
    Q_PROPERTY(int nextStopIndex READ nextStopIndex NOTIFY stopsChanged)
    Q_PROPERTY(QString nextStopName READ nextStopName NOTIFY stopsChanged)
    Q_PROPERTY(double distanceToNextStop READ distanceToNextStop NOTIFY progressChanged)
    Q_PROPERTY(int secondsToNextStop READ secondsToNextStop NOTIFY progressChanged)

    // Direction of travel in degrees, used to rotate the map
    Q_PROPERTY(double heading READ heading NOTIFY progressChanged)
    Q_PROPERTY(QVariantList traveledPath READ traveledPath NOTIFY progressChanged)
    Q_PROPERTY(QVariantList remainingPath READ remainingPath NOTIFY progressChanged)

public:
    NavigationController(LocationProvider *locationProvider,
                         RoutingService *routingService,
                         QObject *parent = nullptr);

    bool navigating() const;
    bool arrived() const;
    bool rerouting() const;
    QString maneuverType() const;
    QString instruction() const;
    double distanceToManeuver() const;
    QString thenManeuverType() const;
    double remainingDistance() const;
    int remainingSeconds() const;
    QVariantList stops() const;
    int stopCount() const;
    int nextStopIndex() const;
    QString nextStopName() const;
    double distanceToNextStop() const;
    int secondsToNextStop() const;
    double heading() const;
    QVariantList traveledPath() const;
    QVariantList remainingPath() const;

    // Starts guidance along the route currently calculated by RoutingService
    Q_INVOKABLE void startNavigation();
    Q_INVOKABLE void stopNavigation();

signals:
    void navigatingChanged(bool navigating);
    void progressChanged();
    void reroutingChanged();
    void stopsChanged();
    // A stop of the trip was reached; the last index is the final destination
    void stopReached(int index);

private:
    void onPositionUpdated(const Position &position);
    void onRouteChanged();
    void startTrip(const QList<Destination> &stops);
    void setReachedStops(int count);
    void updateProgress(const Position &position);
    void setNavigating(bool navigating);
    void checkOffRoute(const Position &position, const RouteTracker::Progress &progress);
    void setRerouting(bool rerouting);
    QString instructionFor(int maneuverIndex) const;
    int legEndingWith(int maneuverIndex) const;

    static QString maneuverKind(int valhallaType);

    LocationProvider *m_locationProvider;
    RoutingService *m_routingService;
    RouteTracker m_tracker;

    // The whole trip; the current route covers m_tripStops.mid(m_routeFirstStop)
    QList<Destination> m_tripStops;
    int m_routeFirstStop = 0;
    int m_reachedStops = 0;
    QVariantList m_stopsList;

    bool m_navigating = false;
    bool m_arrived = false;
    bool m_rerouting = false;
    QElapsedTimer m_offRouteTimer;  // running while the position is off the route
    QElapsedTimer m_lastReroute;
    QString m_maneuverType;
    QString m_instruction;
    double m_distanceToManeuver = 0.0;
    QString m_thenManeuverType;
    double m_remainingDistance = 0.0;
    int m_remainingSeconds = 0;
    double m_distanceToNextStop = 0.0;
    int m_secondsToNextStop = 0;
    double m_heading = 0.0;
    QVariantList m_traveledPath;
    QVariantList m_remainingPath;
};
