import QtQuick
import QtQuick.Effects
import QtQuick.Layouts
import navigation

// Top banner while navigating: next maneuver arrow, distance to it and the
// instruction, plus an optional "Then" strip with the maneuver after it.
Rectangle {
    id: root

    // Maneuver kinds from NavigationController: "straight", "slight-left/right",
    // "left/right", "sharp-left/right", "uturn-left/right", "roundabout",
    // "depart", "arrive"
    property string maneuverType: "straight"
    property real distanceToManeuver: 0 // meters
    property string instruction
    property string thenManeuverType // empty = no "Then" strip
    // Shown instead of the maneuver while a new route is calculated
    property bool rerouting: false

    implicitHeight: content.implicitHeight
    radius: Theme.cardRadius
    color: Theme.maneuver

    layer.enabled: true
    layer.effect: MultiEffect {
        shadowEnabled: true
        shadowColor: "#000000"
        shadowOpacity: 0.3
        shadowBlur: 0.8
        shadowVerticalOffset: 3
    }

    function iconFor(type) {
        switch (type) {
        case "left": case "sharp-left": return Icons.turnLeft
        case "right": case "sharp-right": return Icons.turnRight
        case "uturn-left": return Icons.uturnLeft
        case "uturn-right": return Icons.uturnRight
        case "roundabout": return Icons.roundabout
        case "depart": return Icons.navigation
        case "arrive": return Icons.place
        default: return Icons.straight
        }
    }

    // Slight and sharp turns reuse the straight and turn arrows, tilted
    function rotationFor(type) {
        switch (type) {
        case "slight-left": return -45
        case "slight-right": return 45
        case "sharp-left": return -45
        case "sharp-right": return 45
        default: return 0
        }
    }

    function formatDistance(meters) {
        if (meters >= 1000)
            return (meters / 1000).toFixed(meters >= 10000 ? 0 : 1) + " km"
        if (meters >= 100)
            return Math.round(meters / 50) * 50 + " m"
        return Math.round(meters / 10) * 10 + " m"
    }

    ColumnLayout {
        id: content
        anchors.left: parent.left
        anchors.right: parent.right
        spacing: 0

        RowLayout {
            Layout.fillWidth: true
            Layout.margins: Theme.cardPadding
            spacing: 22

            Icon {
                path: root.rerouting ? Icons.directions : root.iconFor(root.maneuverType)
                size: Theme.iconManeuver
                rotation: root.rerouting ? 0 : root.rotationFor(root.maneuverType)
                color: Theme.textOnAccent
            }

            ColumnLayout {
                Layout.fillWidth: true
                spacing: 2

                Text {
                    text: root.rerouting ? qsTr("Rerouting…")
                                         : root.formatDistance(root.distanceToManeuver)
                    color: Theme.textOnAccent
                    font.pixelSize: Theme.fontDisplay
                    font.weight: Font.Bold
                }

                Text {
                    Layout.fillWidth: true
                    text: root.rerouting ? qsTr("Calculating a new route from your position")
                                         : root.instruction
                    color: Theme.textOnAccent
                    font.pixelSize: Theme.fontTitle
                    wrapMode: Text.WordWrap
                    maximumLineCount: 2
                    elide: Text.ElideRight
                }
            }
        }

        // "Then <arrow>" strip for the maneuver after the next one
        Rectangle {
            visible: !root.rerouting && root.thenManeuverType.length > 0
            Layout.fillWidth: true
            implicitHeight: 56
            color: Theme.maneuverSecondary
            bottomLeftRadius: root.radius
            bottomRightRadius: root.radius

            Row {
                anchors.left: parent.left
                anchors.leftMargin: Theme.cardPadding
                anchors.verticalCenter: parent.verticalCenter
                spacing: 10

                Text {
                    anchors.verticalCenter: parent.verticalCenter
                    text: qsTr("Then")
                    color: Theme.textOnAccent
                    font.pixelSize: Theme.fontBody
                    font.weight: Font.DemiBold
                }

                Icon {
                    anchors.verticalCenter: parent.verticalCenter
                    path: root.iconFor(root.thenManeuverType)
                    size: Theme.iconMedium
                    rotation: root.rotationFor(root.thenManeuverType)
                    color: Theme.textOnAccent
                }
            }
        }
    }
}
