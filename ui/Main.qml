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

    // Debug: show the navigation panels without navigating
    property bool debugNavigationPanels: false
    readonly property bool navigationVisible: controller.navigating || debugNavigationPanels

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
        onStartRouteClicked: {
            // TODO: start navigation along the route created on the map
        }
        onCreateRouteClicked: window.routingService.calculateRoute(
                                  mapView.currentCoordinate,
                                  mapView.destinationCoordinate)
        onClearClicked: mapView.clearDestination()
    }

    // --- Navigating (placeholder data until NavigationController provides it) ---

    ManeuverBanner {
        id: maneuverBanner
        visible: window.navigationVisible
        anchors.top: statusBar.bottom
        anchors.left: parent.left
        anchors.margins: Theme.spacing
        width: Math.min(parent.width - 2 * Theme.spacing, 560)

        maneuverType: "right"
        distanceToManeuver: 300
        instruction: qsTr("Turn right onto Mikołowska")
        thenManeuverType: "left"
    }

    TripSummary {
        id: tripSummary
        visible: window.navigationVisible
        anchors.bottom: parent.bottom
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottomMargin: Theme.spacing
        width: Math.min(parent.width - 2 * Theme.spacing, 640)

        remainingDistance: 5700
        remainingSeconds: 12 * 60

        onEndClicked: {
            window.controller.stopNavigation()
            window.debugNavigationPanels = false
        }
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

    // Debug only: toggles the navigation panels
    FloatingButton {
        anchors.top: statusBar.bottom
        anchors.right: parent.right
        anchors.margins: Theme.spacing
        iconPath: Icons.bug
        iconColor: window.debugNavigationPanels ? Theme.danger : Theme.textSecondary
        onClicked: window.debugNavigationPanels = !window.debugNavigationPanels
    }
}
