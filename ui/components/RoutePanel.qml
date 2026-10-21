import QtQuick
import QtQuick.Effects
import QtQuick.Layouts
import navigation

// Floating card at the bottom of the screen. Without a destination it offers
// navigation with a route received over Bluetooth; with a destination picked
// on the map it offers creating and clearing the route.
Rectangle {
    id: root

    property bool hasDestination: false
    property var destinationCoordinate
    property bool hasRoute: false
    property bool busy: false
    property string errorString
    property bool canCreateRoute: true

    signal startNavigationClicked()
    signal createRouteClicked()
    signal clearClicked()

    readonly property bool hasError: errorString.length > 0

    implicitHeight: content.implicitHeight + 2 * 16
    radius: Theme.cardRadius
    color: Theme.surface

    layer.enabled: true
    layer.effect: MultiEffect {
        shadowEnabled: true
        shadowColor: "#000000"
        shadowOpacity: 0.25
        shadowBlur: 0.8
        shadowVerticalOffset: 3
    }

    function formatCoordinate(coordinate) {
        if (!coordinate || !coordinate.isValid)
            return ""
        const lat = Math.abs(coordinate.latitude).toFixed(4) + "° " + (coordinate.latitude >= 0 ? "N" : "S")
        const lon = Math.abs(coordinate.longitude).toFixed(4) + "° " + (coordinate.longitude >= 0 ? "E" : "W")
        return lat + ", " + lon
    }

    ColumnLayout {
        id: content
        anchors.fill: parent
        anchors.margins: 16
        spacing: 14

        RowLayout {
            Layout.fillWidth: true
            spacing: 12

            // Leading icon in a tinted circle
            Rectangle {
                implicitWidth: 44
                implicitHeight: 44
                radius: width / 2
                color: root.hasError ? Theme.dangerSoft : Theme.accentSoft

                Icon {
                    anchors.centerIn: parent
                    path: root.hasDestination ? Icons.place : Icons.bluetooth
                    size: 24
                    color: root.hasError ? Theme.danger : Theme.accent
                }
            }

            ColumnLayout {
                Layout.fillWidth: true
                spacing: 2

                Text {
                    Layout.fillWidth: true
                    text: root.hasDestination ? qsTr("Selected destination") : qsTr("Where to?")
                    color: Theme.textPrimary
                    font.pixelSize: Theme.fontTitle
                    font.weight: Font.DemiBold
                    elide: Text.ElideRight
                }

                Text {
                    Layout.fillWidth: true
                    text: !root.hasDestination ? qsTr("Send a route from your phone or tap the map")
                        : root.busy ? qsTr("Calculating route…")
                        : root.hasError ? root.errorString
                        : root.hasRoute ? qsTr("Route ready")
                        : root.formatCoordinate(root.destinationCoordinate)
                    color: root.hasError ? Theme.danger : Theme.textSecondary
                    font.pixelSize: Theme.fontBody
                    elide: Text.ElideRight
                }
            }
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 10

            PillButton {
                visible: root.hasDestination
                primary: false
                iconPath: Icons.close
                text: qsTr("Clear")
                onClicked: root.clearClicked()
            }

            PillButton {
                Layout.fillWidth: true
                iconPath: root.hasDestination ? Icons.directions : Icons.navigation
                text: !root.hasDestination ? qsTr("Start navigation")
                    : root.busy ? qsTr("Calculating…")
                    : root.hasRoute ? qsTr("Recalculate route")
                    : qsTr("Create route")
                enabled: !root.hasDestination || (root.canCreateRoute && !root.busy)
                onClicked: root.hasDestination ? root.createRouteClicked()
                                               : root.startNavigationClicked()
            }
        }
    }
}
