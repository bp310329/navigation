#include "location/LocationProvider.h"

#include <QGeoPositionInfo>

LocationProvider::LocationProvider(QGeoPositionInfoSource *source,
                                   QObject *parent)
    : QObject(parent),
      m_source(source) {
    if (!m_source)
        return;

    connect(m_source, &QGeoPositionInfoSource::positionUpdated,
            this, &LocationProvider::onPositionUpdated);
    connect(m_source, &QGeoPositionInfoSource::errorOccurred,
            this, &LocationProvider::onSourceError);
}

bool LocationProvider::locationAvailable() const {
    return m_locationAvailable;
}

Position LocationProvider::position() const {
    return m_position;
}

void LocationProvider::start() {
    if (m_source)
        m_source->startUpdates();
}

void LocationProvider::stop() {
    if (m_source)
        m_source->stopUpdates();
    setLocationAvailable(false);
}

void LocationProvider::onPositionUpdated(const QGeoPositionInfo &info) {
    Position position;
    position.latitude = info.coordinate().latitude();
    position.longitude = info.coordinate().longitude();
    if (info.hasAttribute(QGeoPositionInfo::GroundSpeed))
        position.speed = info.attribute(QGeoPositionInfo::GroundSpeed);
    position.valid = info.isValid();

    m_position = position;
    emit positionUpdated(m_position);
    setLocationAvailable(m_position.valid);
}

void LocationProvider::onSourceError(QGeoPositionInfoSource::Error error) {
    if (error != QGeoPositionInfoSource::NoError)
        setLocationAvailable(false);
}

void LocationProvider::setLocationAvailable(bool available) {
    if (m_locationAvailable == available)
        return;

    m_locationAvailable = available;
    emit locationAvailableChanged(m_locationAvailable);
}
