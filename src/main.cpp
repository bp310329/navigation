#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickWindow>

#include "controller/NavigationController.h"
#include "location/DummyPositionSource.h"
#include "location/KeyboardDriver.h"
#include "location/LocationProvider.h"
#include "routing/RoutingService.h"

int main(int argc, char *argv[])
{
    // MapLibre is built with the OpenGL backend; Qt defaults to Direct3D on Windows.
    QQuickWindow::setGraphicsApi(QSGRendererInterface::OpenGL);

    QGuiApplication app(argc, argv);
    QQmlApplicationEngine engine;

    // Dummy source until real positioning is available. Replace with e.g.
    // QGeoPositionInfoSource::createDefaultSource() or QNmeaPositionInfoSource.
    DummyPositionSource positionSource(QGeoCoordinate(50.2312, 18.9688));

    LocationProvider locationProvider(&positionSource);

    // Valhalla routing server (running locally in WSL)
    RoutingService routingService(QUrl("http://172.18.77.37:8002"));

    NavigationController controller(&locationProvider, &routingService);

    // Testing: drive the dummy position with WASD / arrow keys (Shift = faster).
    // When navigation starts, the simulated car is turned to face along the route.
    KeyboardDriver keyboardDriver(&positionSource);
    app.installEventFilter(&keyboardDriver);
    QObject::connect(&controller, &NavigationController::navigatingChanged,
                     &keyboardDriver, [&](bool navigating) {
        if (navigating)
            keyboardDriver.setHeading(controller.heading());
    });

    engine.setInitialProperties({
        { "controller", QVariant::fromValue(&controller) },
        { "locationProvider", QVariant::fromValue(&locationProvider) },
        { "routingService", QVariant::fromValue(&routingService) },
    });

    locationProvider.start();

    engine.loadFromModule("navigation", "Main");

    return app.exec();
}
