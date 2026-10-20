import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ApplicationWindow {
    id: window
    width: 1280
    height: 720
    visible: true
    title: qsTr("Navigation")

    required property NavigationController controller
    required property LocationProvider locationProvider
    required property RoutingService routingService

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 20
        spacing: 20

        Label {
            text: "NO CONNECTION"
            font.pixelSize: 16
        }

        MapView {
            id: mapView
            Layout.fillWidth: true
            Layout.fillHeight: true

            locationAvailable: window.locationProvider.locationAvailable
            currentCoordinate: window.locationProvider.position.coordinate
            routePath: window.routingService.path

            // A new destination invalidates the old route
            onDestinationCoordinateChanged: window.routingService.clearRoute()
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 20

            Button {
                palette.buttonText: "black"
                text: "Center"
                enabled: mapView.hasLocation
                onClicked: mapView.centerOnCurrentLocation()
            }

            Button {
                palette.buttonText: "black"
                text: "Create Route"
                visible: mapView.hasDestination
                enabled: mapView.hasLocation && !window.routingService.busy
                onClicked: window.routingService.calculateRoute(
                               mapView.currentCoordinate,
                               mapView.destinationCoordinate)
            }

            Button {
                palette.buttonText: "black"
                text: "Clear Destination"
                visible: mapView.hasDestination
                onClicked: mapView.clearDestination()
            }

            Label {
                visible: text.length > 0
                text: window.routingService.busy
                      ? "Calculating route..."
                      : window.routingService.errorString
                color: window.routingService.busy ? "black" : "#d93025"
            }

            Button {
                palette.buttonText: "black"
                text: "Start Navigation"
                onClicked: {
                    window.controller.startNavigation()
                }
            }
        }
    }
}
