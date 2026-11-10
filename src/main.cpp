#include <QCommandLineParser>
#include <QFile>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickWindow>

#include "controller/NavigationController.h"
#include "controller/TripReceiver.h"
#include "location/DummyPositionSource.h"
#include "location/KeyboardDriver.h"
#include "location/LocationProvider.h"
#include "routing/RoutingService.h"

int main(int argc, char *argv[])
{
    // MapLibre is built with the OpenGL backend; Qt defaults to Direct3D on Windows.
    QQuickWindow::setGraphicsApi(QSGRendererInterface::OpenGL);

    QGuiApplication app(argc, argv);

    QCommandLineParser parser;
    parser.addHelpOption();
    const QCommandLineOption tripOption(
        QStringLiteral("trip"),
        QStringLiteral("Testing: load a route with stops from a JSON file, as if sent from the phone."),
        QStringLiteral("file"));
    parser.addOption(tripOption);
    parser.process(app);

    QQmlApplicationEngine engine;

    // Dummy source until real positioning is available. Replace with e.g.
    // QGeoPositionInfoSource::createDefaultSource() or QNmeaPositionInfoSource.
    DummyPositionSource positionSource(QGeoCoordinate(50.2312, 18.9688));

    LocationProvider locationProvider(&positionSource);

    // Valhalla routing server: NAV_VALHALLA_URL environment variable, or the
    // per-device default set at configure time (see CMakeUserPresets.json)
    const QString valhallaUrl = qEnvironmentVariable("NAV_VALHALLA_URL",
                                                     QStringLiteral(NAV_DEFAULT_VALHALLA_URL));
    RoutingService routingService{QUrl(valhallaUrl)};

    NavigationController controller(&locationProvider, &routingService);

    // Routes sent from the phone. TODO: feed it from Bluetooth:
    //   connect(&bluetooth, &BluetoothManager::dataReceived, &tripReceiver, &TripReceiver::receive);
    TripReceiver tripReceiver(&locationProvider, &routingService);

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
        { "tripReceiver", QVariant::fromValue(&tripReceiver) },
    });

    locationProvider.start();

    engine.loadFromModule("navigation", "Main");

    if (parser.isSet(tripOption)) {
        QFile tripFile(parser.value(tripOption));
        if (tripFile.open(QIODevice::ReadOnly))
            tripReceiver.receive(tripFile.readAll());
        else
            qWarning() << "Cannot open trip file" << tripFile.fileName() << tripFile.errorString();
    }

    return app.exec();
}
