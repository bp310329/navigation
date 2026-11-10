#include "controller/TripReceiver.h"

#include <QDebug>

#include "data/DestinationParser.h"
#include "location/LocationProvider.h"
#include "routing/RoutingService.h"

TripReceiver::TripReceiver(LocationProvider *locationProvider,
                           RoutingService *routingService,
                           QObject *parent)
    : QObject(parent),
      m_locationProvider(locationProvider),
      m_routingService(routingService) {
    connect(m_locationProvider, &LocationProvider::positionUpdated,
            this, &TripReceiver::onPositionUpdated);
}

bool TripReceiver::receive(const QByteArray &data) {
    const std::optional<QList<Destination>> stops = DestinationParser::parseStops(data);
    if (!stops) {
        qWarning() << "TripReceiver: ignoring invalid route data";
        emit tripRejected();
        return false;
    }

    m_pendingStops = *stops;
    emit tripReceived(int(m_pendingStops.size()));

    const Position position = m_locationProvider->position();
    if (position.valid)
        calculate(position);
    return true;
}

void TripReceiver::onPositionUpdated(const Position &position) {
    if (!m_pendingStops.isEmpty() && position.valid)
        calculate(position);
}

void TripReceiver::calculate(const Position &position) {
    const QList<Destination> stops = m_pendingStops;
    m_pendingStops.clear();
    m_routingService->calculateRoute(position.coordinate(), stops,
                                     position.hasHeading ? position.heading : -1.0);
}
