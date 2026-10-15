import QtQuick
import QtLocation
import QtPositioning

Item {
    id: root
    clip: true

    // Current device location, bound from LocationProvider by the parent
    property bool locationAvailable: false
    property geoCoordinate currentCoordinate

    readonly property bool hasLocation: locationAvailable && currentCoordinate.isValid
    property bool centeredOnLocation: false

    function centerOnCurrentLocation() {
        if (!hasLocation)
            return
        map.center = currentCoordinate
        centeredOnLocation = true
    }

    // Center on the first fix, then leave the view to the user
    onHasLocationChanged: if (!centeredOnLocation) centerOnCurrentLocation()

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
        center: QtPositioning.coordinate(50.0614, 19.9366)

        property geoCoordinate startCentroid

        // Current location marker
        MapQuickItem {
            id: currentLocationMarker
            visible: root.hasLocation
            coordinate: root.currentCoordinate
            anchorPoint.x: locationDot.width / 2
            anchorPoint.y: locationDot.height / 2

            sourceItem: Rectangle {
                id: locationDot
                width: 20
                height: 20
                radius: width / 2
                color: "#1a73e8"
                border.color: "white"
                border.width: 3
            }
        }

        // Pan with mouse drag or one finger
        DragHandler {
            id: drag
            target: null
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
