#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickWindow>

#include "controller/NavigationController.h"
#include "location/LocationProvider.h"

int main(int argc, char *argv[])
{
    // MapLibre is built with the OpenGL backend; Qt defaults to Direct3D on Windows.
    QQuickWindow::setGraphicsApi(QSGRendererInterface::OpenGL);

    QGuiApplication app(argc, argv);
    QQmlApplicationEngine engine;

    LocationProvider locationProvider;

    NavigationController controller(
        &locationProvider
    );

    engine.rootContext()->setContextProperty(
        "controller",
        &controller
    );

    engine.loadFromModule("navigation", "Main");

    return app.exec();
}
