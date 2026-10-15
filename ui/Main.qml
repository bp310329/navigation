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
                text: "Start Navigation"
                onClicked: {
                    window.controller.startNavigation()
                }
            }
        }
    }
}
