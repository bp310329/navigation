import QtQuick
import QtQuick.Controls
import navigation

ApplicationWindow {
    id: window
    // Official Raspberry Pi 7" touchscreen resolution
    width: 1280
    height: 720
    visible: true
    title: qsTr("Navigation")
    color: Theme.chrome

    required property NavigationController controller
    required property LocationProvider locationProvider
    required property RoutingService routingService

    StatusBar {
        id: statusBar
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        z: 1
    }

    MapView {
        id: mapView
        anchors.top: statusBar.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom

        // Keep the fitted route clear of the floating bottom panel
        bottomInset: routePanel.height + 2 * Theme.spacing

        locationAvailable: window.locationProvider.locationAvailable
        currentCoordinate: window.locationProvider.position.coordinate
        routePath: window.routingService.path

        // A new destination invalidates the old route
        onDestinationCoordinateChanged: window.routingService.clearRoute()
    }

    FloatingButton {
        anchors.right: parent.right
        anchors.bottom: routePanel.top
        anchors.margins: Theme.spacing
        iconPath: Icons.myLocation
        iconColor: mapView.followingLocation ? Theme.accent : Theme.textSecondary
        enabled: mapView.hasLocation
        onClicked: mapView.centerOnCurrentLocation()
    }

    RoutePanel {
        id: routePanel
        anchors.bottom: parent.bottom
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottomMargin: Theme.spacing
        width: Math.min(parent.width - 2 * Theme.spacing, 560)

        hasDestination: mapView.hasDestination
        destinationCoordinate: mapView.destinationCoordinate
        hasRoute: window.routingService.path.length > 1
        busy: window.routingService.busy
        errorString: window.routingService.errorString
        canCreateRoute: mapView.hasLocation

        onStartNavigationClicked: {
            // TODO: start navigation with the route received over Bluetooth
        }
        onCreateRouteClicked: window.routingService.calculateRoute(
                                  mapView.currentCoordinate,
                                  mapView.destinationCoordinate)
        onClearClicked: mapView.clearDestination()
    }
}
