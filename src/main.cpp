#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickWindow>

#include "controller/NavigationController.h"
#include "location/DummyPositionSource.h"
#include "location/LocationProvider.h"

int main(int argc, char *argv[])
{
    // MapLibre is built with the OpenGL backend; Qt defaults to Direct3D on Windows.
    QQuickWindow::setGraphicsApi(QSGRendererInterface::OpenGL);

    QGuiApplication app(argc, argv);
    QQmlApplicationEngine engine;

    // Dummy source until real positioning is available. Replace with e.g.
    // QGeoPositionInfoSource::createDefaultSource() or QNmeaPositionInfoSource.
    DummyPositionSource positionSource(QGeoCoordinate(50.0614, 19.9366));

    LocationProvider locationProvider(&positionSource);
    NavigationController controller(
        &locationProvider
    );

    engine.setInitialProperties({
        { "controller", QVariant::fromValue(&controller) },
        { "locationProvider", QVariant::fromValue(&locationProvider) },
    });

    locationProvider.start();

    engine.loadFromModule("navigation", "Main");

    return app.exec();
}
