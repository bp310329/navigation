#include "controller/NavigationController.h"

#include "location/LocationProvider.h"

NavigationController::NavigationController(
    LocationProvider* locationProvider,
    QObject* parent
)
    : QObject(parent),
      m_locationProvider(locationProvider) {
    // Połączenia sygnałów i slotów między modułami
}

bool NavigationController::navigating() const {
    return m_navigating;
}

void NavigationController::startNavigation(const Destination& destination) {
    // Pobierz aktualną pozycję,
    // zleć wyznaczenie trasy,
    // a po otrzymaniu trasy rozpocznij nawigację.
}

void NavigationController::stopNavigation() {
    // Zatrzymaj prowadzenie po trasie.
}