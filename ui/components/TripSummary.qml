import QtQuick
import QtQuick.Layouts
import navigation

// Bottom card while navigating: remaining time, remaining distance,
// arrival time and a button to end navigation.
Card {
    id: root

    property real remainingDistance: 0 // meters
    property int remainingSeconds: 0

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
