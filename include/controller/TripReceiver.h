#pragma once

#include <QByteArray>
#include <QList>
#include <QObject>
#include <QtQml/qqmlregistration.h>

#include "data/Destination.h"
#include "data/Position.h"

class LocationProvider;
class RoutingService;

// Turns a route sent from the phone into a calculated route. The data is JSON
// with the stops in driving order (format: see DestinationParser). The route
// starts at the current position, so without a position fix the calculation
// waits for the first one.
//
// The phone connection delivers the data, e.g.
//   connect(&bluetooth, &BluetoothManager::dataReceived, &tripReceiver, &TripReceiver::receive);
class TripReceiver : public QObject {
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Provided by the application")

public:
    TripReceiver(LocationProvider *locationProvider,
                 RoutingService *routingService,
                 QObject *parent = nullptr);

public slots:
    // Returns false and ignores the data when it is not a valid route
    bool receive(const QByteArray &data);

signals:
    // A valid route arrived; emitted before its calculation starts
    void tripReceived(int stopCount);
    void tripRejected();

private:
    void onPositionUpdated(const Position &position);
    void calculate(const Position &position);

    LocationProvider *m_locationProvider;
    RoutingService *m_routingService;
    QList<Destination> m_pendingStops; // waiting for a position fix
};
