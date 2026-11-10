import QtQuick
import QtQuick.Layouts
import navigation

// Bottom card while navigating: remaining time, remaining distance,
// arrival time and a button to end navigation. On a route with stops it
// also shows the next stop while it is not the final destination.
Card {
    id: root

    property real remainingDistance: 0 // meters
    property int remainingSeconds: 0

    property int stopCount: 0
    property int nextStopIndex: 0
    property string nextStopName
    property real distanceToNextStop: 0 // meters
    property int secondsToNextStop: 0

    readonly property bool showNextStop: stopCount > 1 && nextStopIndex < stopCount - 1

    signal endClicked()

    implicitHeight: content.implicitHeight + 2 * Theme.cardPadding

    // Refreshed periodically so the arrival time stays current
    property date now: new Date()
    Timer {
        interval: 15000
        running: root.visible
        repeat: true
        triggeredOnStart: true
        onTriggered: root.now = new Date()
    }

    function formatDuration(seconds) {
        const minutes = Math.max(1, Math.round(seconds / 60))
        if (minutes < 60)
            return minutes + " min"
        return Math.floor(minutes / 60) + " h " + (minutes % 60) + " min"
    }

    function formatDistance(meters) {
        if (meters >= 1000)
            return (meters / 1000).toFixed(1) + " km"
        return Math.round(meters) + " m"
    }

    RowLayout {
        id: content
        anchors.fill: parent
        anchors.margins: Theme.cardPadding
        anchors.leftMargin: Theme.cardPadding + 8
        spacing: 18

        ColumnLayout {
            Layout.fillWidth: true
            spacing: 2

            // "Stop 1/3: Spodek · 2.4 km · 6 min"
            Text {
                visible: root.showNextStop
                Layout.fillWidth: true
                text: qsTr("Stop %1/%2").arg(root.nextStopIndex + 1).arg(root.stopCount)
                      + (root.nextStopName.length > 0 ? ": " + root.nextStopName : "")
                      + "  ·  " + root.formatDistance(root.distanceToNextStop)
                      + "  ·  " + root.formatDuration(root.secondsToNextStop)
                color: Theme.accent
                font.pixelSize: Theme.fontSmall
                font.weight: Font.DemiBold
                elide: Text.ElideRight
            }

            Text {
                text: root.formatDuration(root.remainingSeconds)
                color: Theme.eta
                font.pixelSize: Theme.fontHeadline
                font.weight: Font.Bold
            }

            Text {
                text: root.formatDistance(root.remainingDistance) + "  ·  "
                      + qsTr("Arrival %1").arg(Qt.formatTime(
                          new Date(root.now.getTime() + root.remainingSeconds * 1000), "HH:mm"))
                color: Theme.textSecondary
                font.pixelSize: Theme.fontBody
            }
        }

        PillButton {
            iconPath: Icons.close
            text: qsTr("End")
            color: Theme.danger
            pressedColor: Theme.dangerPressed
            onClicked: root.endClicked()
        }
    }
}
