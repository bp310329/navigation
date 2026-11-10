import QtQuick
import QtQuick.Layouts
import navigation

// Floating card at the bottom of the screen. Without a destination it waits
// for a route from the phone (which may have several stops); with a
// destination picked on the map it offers creating, clearing and starting
// the route.
Card {
    id: root

    property bool hasDestination: false
    property var destinationCoordinate
    property bool hasRoute: false
    property bool busy: false
    property string errorString
    property bool canCreateRoute: true
    // Stops of the calculated route: [{ name, coordinate, reached }]
    property var stops: []

    signal startRouteClicked()
    signal createRouteClicked()
    signal clearClicked()

    readonly property bool hasError: errorString.length > 0
    // A route from the phone: calculated, being calculated or failed
    readonly property bool hasPhoneRoute: !hasDestination && (hasRoute || busy || hasError)
    readonly property string finalStopName: stops.length > 0 ? stops[stops.length - 1].name : ""

    implicitHeight: content.implicitHeight + 2 * Theme.cardPadding

    function formatCoordinate(coordinate) {
        if (!coordinate || !coordinate.isValid)
            return ""
        const lat = Math.abs(coordinate.latitude).toFixed(4) + "° " + (coordinate.latitude >= 0 ? "N" : "S")
        const lon = Math.abs(coordinate.longitude).toFixed(4) + "° " + (coordinate.longitude >= 0 ? "E" : "W")
        return lat + ", " + lon
    }

    function routeSummary() {
        if (stops.length > 1)
            return finalStopName.length > 0
                ? qsTr("%1 stops · ends at %2").arg(stops.length).arg(finalStopName)
                : qsTr("%1 stops").arg(stops.length)
        return finalStopName.length > 0 ? finalStopName : qsTr("Route ready")
    }

    ColumnLayout {
        id: content
        anchors.fill: parent
        anchors.margins: Theme.cardPadding
        spacing: 14

        RowLayout {
            Layout.fillWidth: true
            spacing: 16

            // Leading icon in a tinted circle
            Rectangle {
                implicitWidth: 52
                implicitHeight: 52
                radius: width / 2
                color: root.hasError ? Theme.dangerSoft : Theme.accentSoft

                Icon {
                    anchors.centerIn: parent
                    path: root.hasDestination ? Icons.place
                        : root.hasPhoneRoute ? Icons.directions
                        : Icons.bluetooth
                    size: Theme.iconSmall
                    color: root.hasError ? Theme.danger : Theme.accent
                }
            }

            ColumnLayout {
                Layout.fillWidth: true
                spacing: 2

                Text {
                    Layout.fillWidth: true
                    text: root.hasDestination ? qsTr("Selected destination")
                        : root.hasPhoneRoute ? qsTr("Route from phone")
                        : qsTr("Where to?")
                    color: Theme.textPrimary
                    font.pixelSize: Theme.fontButton
                    font.weight: Font.DemiBold
                    elide: Text.ElideRight
                }

                Text {
                    Layout.fillWidth: true
                    text: root.busy ? qsTr("Calculating route…")
                        : root.hasError ? root.errorString
                        : root.hasRoute ? root.routeSummary()
                        : root.hasDestination ? root.formatCoordinate(root.destinationCoordinate)
                        : qsTr("Send a route from your phone or tap the map")
                    color: root.hasError ? Theme.danger : Theme.textSecondary
                    font.pixelSize: Theme.fontBody
                    elide: Text.ElideRight
                }
            }
        }

        RowLayout {
            visible: root.hasDestination || root.hasPhoneRoute
            Layout.fillWidth: true
            spacing: 14

            PillButton {
                primary: false
                iconPath: Icons.close
                text: qsTr("Clear")
                onClicked: root.clearClicked()
            }

            // Create the route to the map destination, or recalculate it once it exists
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

            PillButton {
                visible: root.hasRoute
                Layout.fillWidth: true
                iconPath: Icons.navigation
                text: qsTr("Start navigation")
                enabled: !root.busy
                onClicked: root.startRouteClicked()
            }
        }
    }
}
