import QtQuick
import QtQuick.Layouts
import navigation

// Thin bar at the top: Bluetooth status on the left, clock and battery on the right
Rectangle {
    id: root

    // Placeholders until BluetoothManager and a battery source are wired in
    property bool bluetoothConnected: false
    property string bluetoothDeviceName
    property int batteryLevel: 76

    implicitHeight: Theme.statusBarHeight
    color: Theme.chrome

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: 20
        anchors.rightMargin: 20
        spacing: 12

        Icon {
            path: root.bluetoothConnected ? Icons.bluetooth : Icons.bluetoothDisabled
            size: Theme.iconSmall
            color: root.bluetoothConnected ? Theme.accentOnDark : Theme.textOnDarkMuted
        }

        Text {
            text: root.bluetoothConnected
                  ? (root.bluetoothDeviceName || qsTr("Connected"))
                  : qsTr("Not connected")
            color: root.bluetoothConnected ? Theme.textOnDark : Theme.textOnDarkMuted
            font.pixelSize: Theme.fontSmall
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
