#pragma once

#include <QObject>
#include <QtQml/qqmlregistration.h>
#include <QGeoPositionInfoSource>

#include "data/Position.h"

class QGeoPositionInfo;

// Exposes the current device position to the rest of the application.
// Positions come from any QGeoPositionInfoSource (dummy, NMEA/serial GPS,
// platform default), so the backend can be swapped without changing users.
class LocationProvider : public QObject {
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Provided by the application")

    Q_PROPERTY(bool locationAvailable
               READ locationAvailable
               NOTIFY locationAvailableChanged)

    Q_PROPERTY(Position position
               READ position
               NOTIFY positionUpdated)

public:
    explicit LocationProvider(QGeoPositionInfoSource *source,
                              QObject *parent = nullptr);

    Q_INVOKABLE bool locationAvailable() const;
    Q_INVOKABLE Position position() const;

    Q_INVOKABLE void start();
    Q_INVOKABLE void stop();

signals:
    void positionUpdated(const Position &position);
    void locationAvailableChanged(bool available);

private slots:
    void onPositionUpdated(const QGeoPositionInfo &info);
    void onSourceError(QGeoPositionInfoSource::Error error);

private:
    void setLocationAvailable(bool available);

    QGeoPositionInfoSource *m_source;
    bool m_locationAvailable = false;
    Position m_position;
};
