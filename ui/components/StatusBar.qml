import QtQuick
import QtQuick.Layouts
import navigation

// Thin bar at the top: Bluetooth status on the left (tap to manage the
// connection), clock and battery on the right
Rectangle {
    id: root

    property int bluetoothState: BluetoothManager.Disconnected
    property string bluetoothDeviceName
    // Placeholder until a battery source is wired in
    property int batteryLevel: 76

    signal bluetoothClicked()

    readonly property bool bluetoothConnected: bluetoothState === BluetoothManager.Connected

    implicitHeight: Theme.statusBarHeight
    color: Theme.chrome

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: 8
        anchors.rightMargin: 20
        spacing: 12

        // Bluetooth status, tappable
        Rectangle {
            Layout.fillHeight: true
            Layout.topMargin: 6
            Layout.bottomMargin: 6
            implicitWidth: bluetoothRow.implicitWidth + 24
            radius: height / 2
            color: bluetoothArea.pressed ? Qt.alpha(Theme.textOnDark, 0.16) : "transparent"

            Row {
                id: bluetoothRow
                anchors.centerIn: parent
                spacing: 10

                Icon {
                    anchors.verticalCenter: parent.verticalCenter
                    path: root.bluetoothState === BluetoothManager.Disconnected
                          ? Icons.bluetoothDisabled : Icons.bluetooth
                    size: Theme.iconSmall
                    color: root.bluetoothConnected ? Theme.accentOnDark : Theme.textOnDarkMuted
                }

                Text {
                    anchors.verticalCenter: parent.verticalCenter
                    text: root.bluetoothConnected
                          ? (root.bluetoothDeviceName || qsTr("Connected"))
                          : root.bluetoothState === BluetoothManager.Connecting
                            ? qsTr("Connecting…")
                            : qsTr("Not connected")
                    color: root.bluetoothConnected ? Theme.textOnDark : Theme.textOnDarkMuted
                    font.pixelSize: Theme.fontSmall
                }
            }

            MouseArea {
                id: bluetoothArea
                anchors.fill: parent
                onClicked: root.bluetoothClicked()
            }
        }

        Item { Layout.fillWidth: true }

        Text {
            id: clock
            text: Qt.formatTime(new Date(), "HH:mm")
            color: Theme.textOnDark
            font.pixelSize: Theme.fontBody
            font.weight: Font.DemiBold

            Timer {
                interval: 1000
                running: true
                repeat: true
                onTriggered: clock.text = Qt.formatTime(new Date(), "HH:mm")
            }
        }

        Item { implicitWidth: 8 }

        Text {
            text: root.batteryLevel + "%"
            color: Theme.textOnDark
            font.pixelSize: Theme.fontSmall
        }

        BatteryIndicator {
            level: root.batteryLevel
        }
    }
}
