#include "location/LocationProvider.h"

LocationProvider::LocationProvider(QObject *parent)
    : QObject(parent) {
}

bool LocationProvider::locationAvailable() const {
    return m_locationAvailable;
}

Position LocationProvider::position() const {
    return m_position;
}
