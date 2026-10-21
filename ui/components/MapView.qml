import QtQuick
import QtLocation
import QtPositioning
import navigation

Item {
    id: root
    clip: true

    // Current device location, bound from LocationProvider by the parent
    property bool locationAvailable: false
    property geoCoordinate currentCoordinate

    readonly property bool hasLocation: locationAvailable && currentCoordinate.isValid
    property bool centeredOnLocation: false

    // Map keeps the current location centered until the user moves it
    property bool followingLocation: false

    // Height covered by floating UI at the bottom, kept free when fitting the route
    property real bottomInset: 0

    // Destination placed by tapping the map
    property geoCoordinate destinationCoordinate
    readonly property bool hasDestination: destinationCoordinate.isValid

    // Calculated route geometry, bound from RoutingService by the parent
    property var routePath: []

    // Turn-by-turn guidance, bound from NavigationController by the parent.
    // While navigating the map follows the position rotated to `heading`,
    // and the traveled part of the route is drawn in gray.
    property bool navigating: false
    property real heading: 0
    property var traveledPath: []
    property var remainingPath: []

    readonly property real navigationZoom: 17
    readonly property real navigationTilt: 40

    function clearDestination() {
        destinationCoordinate = QtPositioning.coordinate()
    }

    function centerOnCurrentLocation() {
        if (!hasLocation)
            return
        if (navigating)
            map.zoomLevel = navigationZoom
        centeredOnLocation = true
        followingLocation = true
        updateCamera()
    }

    function updateCamera() {
        if (!hasLocation)
            return
        if (!navigating) {
            map.center = currentCoordinate
            return
        }
        // Driving direction points up, with the position in the lower part
        // of the screen so more of the road ahead is visible
        map.bearing = heading
        map.tilt = navigationTilt
        map.alignCoordinateToPoint(currentCoordinate,
                                   Qt.point(width / 2, height - bottomInset - 120))
    }

    function fitRoute() {
        followingLocation = false
        map.fitViewportToMapItems([routeLine])

        // Fit the route into the free area between the pin (top) and the
        // bottom panel instead of the whole map, plus a small margin
        const topInset = destinationPin.height
        const freeHeight = Math.max(1, height - bottomInset - topInset)
        map.zoomLevel -= Math.log2(height / freeHeight) + 0.2
        map.pan(0, (bottomInset - topInset) / 2)
    }

    // Center on the first fix, then leave the view to the user
    onHasLocationChanged: if (!centeredOnLocation) centerOnCurrentLocation()
    onCurrentCoordinateChanged: if (followingLocation) updateCamera()
    onHeadingChanged: if (followingLocation) updateCamera()

    onNavigatingChanged: {
        if (navigating) {
            map.zoomLevel = navigationZoom
            followingLocation = true
            updateCamera()
        } else {
            map.bearing = 0
            map.tilt = 0
            if (routePath.length > 1)
                fitRoute()
        }
    }

    // Show the whole route once it arrives (but not when rerouting while navigating)
    onRoutePathChanged: if (routePath.length > 1 && !navigating) Qt.callLater(fitRoute)

    Plugin {
        id: mapPlugin
        name: "maplibre"

        PluginParameter {
            name: "maplibre.map.styles"
            value: "https://tiles.openfreemap.org/styles/bright"
        }
    }

    Map {
        id: map
        anchors.fill: parent

        plugin: mapPlugin
        zoomLevel: 12
        center: QtPositioning.coordinate(50.2312, 18.9688)

        property geoCoordinate startCentroid

        // Already traveled part of the route while navigating
        MapPolyline {
            visible: root.navigating && root.traveledPath.length > 1
            path: root.traveledPath
            line.width: 14
            line.color: Theme.routeTraveledCasing
        }
        MapPolyline {
            visible: root.navigating && root.traveledPath.length > 1
            path: root.traveledPath
            line.width: 9
            line.color: Theme.routeTraveled
        }

        // Route ahead (the whole route when not navigating): darker casing under the main line
        MapPolyline {
            visible: routeLine.visible
            path: routeLine.path
            line.width: 14
            line.color: Theme.accentPressed
        }
        MapPolyline {
            id: routeLine
            path: root.navigating ? root.remainingPath : root.routePath
            visible: path.length > 1
            line.width: 9
            line.color: Theme.accent
        }

        // Destination marker
        MapQuickItem {
            id: destinationMarker
            visible: root.hasDestination
            coordinate: root.destinationCoordinate
            anchorPoint.x: destinationPin.width / 2
            // Tip of the pin is at y = 22 of the 24x24 icon
            anchorPoint.y: destinationPin.height * 22 / 24

            sourceItem: Icon {
                id: destinationPin
                path: Icons.place
                size: 72
                color: Theme.danger
                strokeColor: "white"
                strokeWidth: 1.2
            }
        }

        // Current location marker
        MapQuickItem {
            id: currentLocationMarker
            visible: root.hasLocation
            coordinate: root.currentCoordinate
            anchorPoint.x: locationMarker.width / 2
            anchorPoint.y: locationMarker.height / 2

            sourceItem: Item {
                id: locationMarker
                width: 72
                height: 72

                // Browsing: dot with a soft halo
                Rectangle {
                    visible: !root.navigating
                    anchors.fill: parent
                    radius: width / 2
                    color: Qt.alpha(Theme.accent, 0.18)

                    Rectangle {
                        anchors.centerIn: parent
                        width: 32
                        height: 32
                        radius: width / 2
                        color: Theme.accent
                        border.color: "white"
                        border.width: 4
                    }
                }

                // Navigating: arrow pointing in the direction of travel
                Icon {
                    visible: root.navigating
                    anchors.centerIn: parent
                    path: Icons.navigation
                    size: 64
                    color: Theme.accent
                    strokeColor: "white"
                    strokeWidth: 1.5
                    rotation: root.heading - map.bearing
                }
            }
        }

        // Tap / click to place the destination
        TapHandler {
            id: tap
            enabled: !root.navigating
            onTapped: (eventPoint) => {
                root.destinationCoordinate = map.toCoordinate(eventPoint.position, false)
            }
        }

        // Pan with mouse drag or one finger
        DragHandler {
            id: drag
            target: null
            onActiveChanged: if (active) root.followingLocation = false
            onTranslationChanged: (delta) => map.pan(-delta.x, -delta.y)
        }

        // Zoom with mouse wheel, keeping the point under the cursor in place
        WheelHandler {
            id: wheel
            target: null
            acceptedDevices: PointerDevice.Mouse | PointerDevice.TouchPad
            onWheel: (event) => {
                const position = wheel.point.position
                const coordinate = map.toCoordinate(position, false)
                map.zoomLevel += event.angleDelta.y / 120
                map.alignCoordinateToPoint(coordinate, position)
            }
        }

        // Zoom and rotate with two fingers (touchscreen / touchpad pinch)
        PinchHandler {
            id: pinch
            target: null
            grabPermissions: PointerHandler.TakeOverForbidden

            onActiveChanged: if (active) {
                root.followingLocation = false
                map.startCentroid = map.toCoordinate(pinch.centroid.position, false)
            }
            onScaleChanged: (delta) => {
                map.zoomLevel += Math.log2(delta)
                map.alignCoordinateToPoint(map.startCentroid, pinch.centroid.position)
            }
            onRotationChanged: (delta) => {
                map.bearing -= delta
                map.alignCoordinateToPoint(map.startCentroid, pinch.centroid.position)
            }
        }

        // Keyboard zoom (Ctrl + / Ctrl -)
        Shortcut {
            enabled: map.zoomLevel < map.maximumZoomLevel
            sequences: [StandardKey.ZoomIn]
            onActivated: map.zoomLevel = Math.round(map.zoomLevel + 1)
        }
        Shortcut {
            enabled: map.zoomLevel > map.minimumZoomLevel
            sequences: [StandardKey.ZoomOut]
            onActivated: map.zoomLevel = Math.round(map.zoomLevel - 1)
        }
    }
}
