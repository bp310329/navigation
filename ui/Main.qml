import QtQuick
import QtQuick.Controls
import navigation

ApplicationWindow {
    id: window
    // 7" touchscreen resolution
    width: 1280
    height: 720
    visible: true
    title: qsTr("Navigation")
    color: Theme.chrome

    required property NavigationController controller
    required property LocationProvider locationProvider
    required property RoutingService routingService

    readonly property bool navigationVisible: controller.navigating

    // The bottom card currently on screen
    readonly property Item bottomPanel: navigationVisible ? tripSummary : routePanel

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
        bottomInset: window.bottomPanel.height + 2 * Theme.spacing

        locationAvailable: window.locationProvider.locationAvailable
        currentCoordinate: window.locationProvider.position.coordinate
        routePath: window.routingService.path

        navigating: window.controller.navigating
        heading: window.controller.heading
        traveledPath: window.controller.traveledPath
        remainingPath: window.controller.remainingPath

        // A new destination invalidates the old route
        onDestinationCoordinateChanged: window.routingService.clearRoute()
    }

    // --- Browsing: pick a destination or wait for a route over Bluetooth ---

    RoutePanel {
        id: routePanel
        visible: !window.navigationVisible
        anchors.bottom: parent.bottom
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottomMargin: Theme.spacing
        width: Math.min(parent.width - 2 * Theme.spacing, 800)

        hasDestination: mapView.hasDestination
        destinationCoordinate: mapView.destinationCoordinate
        hasRoute: window.routingService.path.length > 1
        busy: window.routingService.busy
        errorString: window.routingService.errorString
        canCreateRoute: mapView.hasLocation

        onStartNavigationClicked: {
            // TODO: start navigation with the route received over Bluetooth
        }
        onStartRouteClicked: window.controller.startNavigation()
        onCreateRouteClicked: window.routingService.calculateRoute(
                                  mapView.currentCoordinate,
                                  mapView.destinationCoordinate)
        onClearClicked: mapView.clearDestination()
    }

    // --- Navigating ---

    ManeuverBanner {
        id: maneuverBanner
        visible: window.navigationVisible
        anchors.top: statusBar.bottom
        anchors.left: parent.left
        anchors.margins: Theme.spacing
        width: Math.min(parent.width - 2 * Theme.spacing, 560)

        maneuverType: window.controller.maneuverType
        distanceToManeuver: window.controller.distanceToManeuver
        instruction: window.controller.instruction
        thenManeuverType: window.controller.thenManeuverType
        rerouting: window.controller.rerouting
    }

    TripSummary {
        id: tripSummary
        visible: window.navigationVisible
        anchors.bottom: parent.bottom
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottomMargin: Theme.spacing
        width: Math.min(parent.width - 2 * Theme.spacing, 640)

        remainingDistance: window.controller.remainingDistance
        remainingSeconds: window.controller.remainingSeconds

        onEndClicked: window.controller.stopNavigation()
    }

    // --- Floating map buttons ---

    FloatingButton {
        anchors.right: parent.right
        anchors.bottom: window.bottomPanel.top
        anchors.margins: Theme.spacing
        iconPath: Icons.myLocation
        iconColor: mapView.followingLocation ? Theme.accent : Theme.textSecondary
        enabled: mapView.hasLocation
        onClicked: mapView.centerOnCurrentLocation()
    }
}
