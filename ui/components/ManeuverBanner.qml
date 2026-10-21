import QtQuick
import QtQuick.Effects
import QtQuick.Layouts
import navigation

// Top banner while navigating: next maneuver arrow, distance to it and the
// instruction, plus an optional "Then" strip with the maneuver after it.
Rectangle {
    id: root

    // Maneuver kinds: "left", "right", "straight", "arrive"
    property string maneuverType: "right"
    property real distanceToManeuver: 0 // meters
    property string instruction
    property string thenManeuverType // empty = no "Then" strip

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
        case "left": return Icons.turnLeft
        case "right": return Icons.turnRight
        case "arrive": return Icons.place
        default: return Icons.straight
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
                path: root.iconFor(root.maneuverType)
                size: Theme.iconManeuver
                color: Theme.textOnAccent
            }

            ColumnLayout {
                Layout.fillWidth: true
                spacing: 2

                Text {
                    text: root.formatDistance(root.distanceToManeuver)
                    color: Theme.textOnAccent
                    font.pixelSize: Theme.fontDisplay
                    font.weight: Font.Bold
                }

                Text {
                    Layout.fillWidth: true
                    text: root.instruction
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
            visible: root.thenManeuverType.length > 0
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
                    color: Theme.textOnAccent
                }
            }
        }
    }
}
