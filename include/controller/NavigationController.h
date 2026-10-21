#pragma once

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
class NavigationController : public QObject {
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Provided by the application")

    Q_PROPERTY(bool navigating READ navigating NOTIFY navigatingChanged)
    Q_PROPERTY(bool arrived READ arrived NOTIFY progressChanged)

    // Next maneuver: kind for the icon ("left", "right", "slight-right",
    // "roundabout", "arrive", ...), instruction text and distance to it
    Q_PROPERTY(QString maneuverType READ maneuverType NOTIFY progressChanged)
    Q_PROPERTY(QString instruction READ instruction NOTIFY progressChanged)
    Q_PROPERTY(double distanceToManeuver READ distanceToManeuver NOTIFY progressChanged)
    // Kind of the maneuver after the next one when it follows shortly, else empty
    Q_PROPERTY(QString thenManeuverType READ thenManeuverType NOTIFY progressChanged)

    Q_PROPERTY(double remainingDistance READ remainingDistance NOTIFY progressChanged)
    Q_PROPERTY(int remainingSeconds READ remainingSeconds NOTIFY progressChanged)

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
    QString maneuverType() const;
    QString instruction() const;
    double distanceToManeuver() const;
    QString thenManeuverType() const;
    double remainingDistance() const;
    int remainingSeconds() const;
    double heading() const;
    QVariantList traveledPath() const;
    QVariantList remainingPath() const;

    // Starts guidance along the route currently calculated by RoutingService
    Q_INVOKABLE void startNavigation();
    Q_INVOKABLE void stopNavigation();

signals:
    void navigatingChanged(bool navigating);
    void progressChanged();

private:
    void onPositionUpdated(const Position &position);
    void onRouteChanged();
    void updateProgress(const Position &position);
    void setNavigating(bool navigating);

    static QString maneuverKind(int valhallaType);

    LocationProvider *m_locationProvider;
    RoutingService *m_routingService;
    RouteTracker m_tracker;

    bool m_navigating = false;
    bool m_arrived = false;
    QString m_maneuverType;
    QString m_instruction;
    double m_distanceToManeuver = 0.0;
    QString m_thenManeuverType;
    double m_remainingDistance = 0.0;
    int m_remainingSeconds = 0;
    double m_heading = 0.0;
    QVariantList m_traveledPath;
    QVariantList m_remainingPath;
};
