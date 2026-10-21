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

    function clearDestination() {
        destinationCoordinate = QtPositioning.coordinate()
    }

    function centerOnCurrentLocation() {
        if (!hasLocation)
            return
        map.center = currentCoordinate
        centeredOnLocation = true
        followingLocation = true
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
    onCurrentCoordinateChanged: if (followingLocation && hasLocation) map.center = currentCoordinate

    // Show the whole route once it arrives
    onRoutePathChanged: if (routePath.length > 1) Qt.callLater(fitRoute)

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

        // Calculated route: darker casing under the main line
        MapPolyline {
            visible: root.routePath.length > 1
            path: root.routePath
            line.width: 14
            line.color: Theme.accentPressed
        }
        MapPolyline {
            id: routeLine
            visible: root.routePath.length > 1
            path: root.routePath
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
            anchorPoint.x: locationHalo.width / 2
            anchorPoint.y: locationHalo.height / 2

            sourceItem: Rectangle {
                id: locationHalo
                width: 72
                height: 72
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
        }

        // Tap / click to place the destination
        TapHandler {
            id: tap
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
