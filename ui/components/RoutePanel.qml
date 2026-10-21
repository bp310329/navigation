import QtQuick
import QtQuick.Layouts
import navigation

// Floating card at the bottom of the screen. Without a destination it offers
// navigation with a route received over Bluetooth; with a destination picked
// on the map it offers creating, clearing and starting the route.
Card {
    id: root

    property bool hasDestination: false
    property var destinationCoordinate
    property bool hasRoute: false
    property bool busy: false
    property string errorString
    property bool canCreateRoute: true

    signal startNavigationClicked()     // route received over Bluetooth
    signal startRouteClicked()          // route created from the map destination
    signal createRouteClicked()
    signal clearClicked()

    readonly property bool hasError: errorString.length > 0

    implicitHeight: content.implicitHeight + 2 * Theme.cardPadding

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
        anchors.margins: Theme.cardPadding
        spacing: 20

        RowLayout {
            Layout.fillWidth: true
            spacing: 18

            // Leading icon in a tinted circle
            Rectangle {
                implicitWidth: 64
                implicitHeight: 64
                radius: width / 2
                color: root.hasError ? Theme.dangerSoft : Theme.accentSoft

                Icon {
                    anchors.centerIn: parent
                    path: root.hasDestination ? Icons.place : Icons.bluetooth
                    size: Theme.iconMedium
                    color: root.hasError ? Theme.danger : Theme.accent
                }
            }

            ColumnLayout {
                Layout.fillWidth: true
                spacing: 4

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
            spacing: 14

            PillButton {
                visible: root.hasDestination
                primary: false
                iconPath: Icons.close
                text: qsTr("Clear")
                onClicked: root.clearClicked()
            }

            // Create route, or recalculate it once it exists
            PillButton {
                visible: root.hasDestination
                Layout.fillWidth: !root.hasRoute
                primary: !root.hasRoute
                iconPath: Icons.directions
                text: root.busy ? qsTr("Calculating…")
                    : root.hasRoute ? qsTr("Recalculate")
                    : qsTr("Create route")
                enabled: root.canCreateRoute && !root.busy
                onClicked: root.createRouteClicked()
            }

            // Start navigating: over Bluetooth without a destination,
            // along the created route with one
            PillButton {
                visible: !root.hasDestination || root.hasRoute
                Layout.fillWidth: true
                iconPath: Icons.navigation
                text: qsTr("Start navigation")
                enabled: !root.busy
                onClicked: root.hasDestination ? root.startRouteClicked()
                                               : root.startNavigationClicked()
            }
        }
    }
}
