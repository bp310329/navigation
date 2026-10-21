#include "location/DummyPositionSource.h"

#include <QDateTime>

namespace {
constexpr int DefaultUpdateIntervalMs = 1000;
constexpr int MinimumUpdateIntervalMs = 100;
}

DummyPositionSource::DummyPositionSource(const QGeoCoordinate &coordinate,
                                         QObject *parent)
    : QGeoPositionInfoSource(parent),
      m_coordinate(coordinate) {
    m_timer.setInterval(DefaultUpdateIntervalMs);
    connect(&m_timer, &QTimer::timeout, this, &DummyPositionSource::emitPosition);
}

void DummyPositionSource::setCoordinate(const QGeoCoordinate &coordinate) {
    m_coordinate = coordinate;
}

void DummyPositionSource::setUpdateInterval(int msec) {
    QGeoPositionInfoSource::setUpdateInterval(msec);
    m_timer.setInterval(msec > 0 ? qMax(msec, MinimumUpdateIntervalMs)
                                 : DefaultUpdateIntervalMs);
}

QGeoPositionInfo DummyPositionSource::lastKnownPosition(bool) const {
    return m_lastPosition;
}

QGeoPositionInfoSource::PositioningMethods
DummyPositionSource::supportedPositioningMethods() const {
    return AllPositioningMethods;
}

int DummyPositionSource::minimumUpdateInterval() const {
    return MinimumUpdateIntervalMs;
}

QGeoPositionInfoSource::Error DummyPositionSource::error() const {
    return NoError;
}

void DummyPositionSource::startUpdates() {
    if (m_timer.isActive())
        return;

    m_timer.start();
    // Report immediately instead of waiting for the first tick.
    QTimer::singleShot(0, this, &DummyPositionSource::emitPosition);
}

void DummyPositionSource::stopUpdates() {
    m_timer.stop();
}

void DummyPositionSource::requestUpdate(int) {
    QTimer::singleShot(0, this, &DummyPositionSource::emitPosition);
}

void DummyPositionSource::emitPosition() {
    QGeoPositionInfo info(m_coordinate, QDateTime::currentDateTimeUtc());
    info.setAttribute(QGeoPositionInfo::GroundSpeed, 0.0);

    m_lastPosition = info;
    emit positionUpdated(info);
}
