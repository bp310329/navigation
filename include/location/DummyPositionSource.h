#pragma once

#include <QGeoCoordinate>
#include <QGeoPositionInfoSource>
#include <QTimer>

// Position source that reports a fixed coordinate at a regular interval.
// Stand-in for a real GPS source until one is available.
class DummyPositionSource : public QGeoPositionInfoSource {
    Q_OBJECT

public:
    explicit DummyPositionSource(const QGeoCoordinate &coordinate,
                                 QObject *parent = nullptr);

    // Moves the reported position, e.g. to simulate driving
    void setCoordinate(const QGeoCoordinate &coordinate);

    void setUpdateInterval(int msec) override;
    QGeoPositionInfo lastKnownPosition(
        bool fromSatellitePositioningMethodsOnly = false) const override;
    PositioningMethods supportedPositioningMethods() const override;
    int minimumUpdateInterval() const override;
    Error error() const override;

public slots:
    void startUpdates() override;
    void stopUpdates() override;
    void requestUpdate(int timeout = 0) override;

private:
    void emitPosition();

    QGeoCoordinate m_coordinate;
    QGeoPositionInfo m_lastPosition;
    QTimer m_timer;
};
