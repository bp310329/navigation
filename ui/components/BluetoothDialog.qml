pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import navigation

// Modal card for the phone connection: the current connection state and the
// list of nearby Bluetooth devices to connect to
Popup {
    id: root

    required property BluetoothManager bluetooth

    readonly property int connectionState: bluetooth.state
    readonly property bool connected: connectionState === BluetoothManager.Connected
    readonly property bool connecting: connectionState === BluetoothManager.Connecting

    modal: true
    dim: true
    padding: Theme.cardPadding
    anchors.centerIn: Overlay.overlay
    width: Math.min(parent.width - 2 * Theme.spacing, 680)
    height: Math.min(parent.height - 2 * Theme.spacing, 600)

    // Look for devices right away, unless already connected
    onOpened: if (!connected && !connecting) bluetooth.startDiscovery()
    onClosed: bluetooth.stopDiscovery()

    Overlay.modal: Rectangle {
        color: Qt.alpha("#000000", 0.45)
    }

    background: Rectangle {
        radius: Theme.cardRadius
        color: Theme.surface
    }

    contentItem: ColumnLayout {
        spacing: 16

        // Header
        RowLayout {
            Layout.fillWidth: true
            spacing: 16

            Text {
                Layout.fillWidth: true
                text: qsTr("Phone connection")
                color: Theme.textPrimary
                font.pixelSize: Theme.fontTitle
                font.weight: Font.DemiBold
            }

            PillButton {
                primary: false
                iconPath: Icons.close
                onClicked: root.close()
            }
        }

        // Connection state
        Rectangle {
            Layout.fillWidth: true
            implicitHeight: statusRow.implicitHeight + 2 * 16
            radius: 20
            color: root.bluetooth.errorString.length > 0 && !root.connected
                   ? Theme.dangerSoft : Theme.accentSoft

            RowLayout {
                id: statusRow
                anchors.fill: parent
                anchors.margins: 16
                anchors.leftMargin: 20
                spacing: 16

                Icon {
                    path: root.connected || root.connecting ? Icons.bluetooth : Icons.bluetoothDisabled
                    size: Theme.iconMedium
                    color: root.bluetooth.errorString.length > 0 && !root.connected
                           ? Theme.danger : Theme.accent
                }

                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 2

                    Text {
                        Layout.fillWidth: true
                        text: root.connected ? qsTr("Connected to %1").arg(root.bluetooth.deviceName)
                            : root.connecting ? qsTr("Connecting to %1…").arg(root.bluetooth.deviceName)
                            : qsTr("Not connected")
                        color: Theme.textPrimary
                        font.pixelSize: Theme.fontBody
                        font.weight: Font.DemiBold
                        elide: Text.ElideRight
                    }

                    Text {
                        Layout.fillWidth: true
                        text: root.connected ? qsTr("Waiting for a route from the phone")
                            : root.connecting ? qsTr("Open the navigation app on the phone")
                            : root.bluetooth.errorString.length > 0 ? root.bluetooth.errorString
                            : qsTr("Pick your phone from the list below")
                        color: root.bluetooth.errorString.length > 0 && !root.connected
                               ? Theme.danger : Theme.textSecondary
                        font.pixelSize: Theme.fontSmall
                        wrapMode: Text.WordWrap
                        maximumLineCount: 2
                        elide: Text.ElideRight
                    }
                }

                PillButton {
                    visible: root.connected || root.connecting
                    primary: false
                    text: root.connected ? qsTr("Disconnect") : qsTr("Cancel")
                    onClicked: root.bluetooth.disconnectFromDevice()
                }
            }
        }

        // Device list header
        RowLayout {
            Layout.fillWidth: true
            Layout.topMargin: 4
            spacing: 12

            Text {
                Layout.fillWidth: true
                text: root.bluetooth.discovering ? qsTr("Searching for devices…")
                                                 : qsTr("Available devices")
                color: Theme.textSecondary
                font.pixelSize: Theme.fontSmall
                font.weight: Font.DemiBold
            }

            PillButton {
                primary: false
                text: root.bluetooth.discovering ? qsTr("Stop") : qsTr("Search")
                onClicked: root.bluetooth.discovering ? root.bluetooth.stopDiscovery()
                                                      : root.bluetooth.startDiscovery()
            }
        }

        ListView {
            id: deviceList
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            spacing: 6
            model: root.bluetooth.devices
            boundsBehavior: Flickable.StopAtBounds

            delegate: Rectangle {
                id: deviceRow

                required property var modelData

                readonly property bool isCurrent: modelData.address === root.bluetooth.deviceAddress
                                                  && (root.connected || root.connecting)

                width: ListView.view.width
                height: Theme.touchTarget
                radius: 20
                color: deviceArea.pressed ? Theme.surfacePressed
                     : isCurrent ? Theme.accentSoft
                     : "transparent"
                border.color: Theme.outline
                border.width: isCurrent ? 0 : 2

                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: 20
                    anchors.rightMargin: 24
                    spacing: 16

                    Icon {
                        path: deviceRow.modelData.isPhone ? Icons.smartphone : Icons.bluetooth
                        size: Theme.iconMedium
                        color: Theme.accent
                    }

                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 0

                        Text {
                            Layout.fillWidth: true
                            text: deviceRow.modelData.name || qsTr("Unknown device")
                            color: Theme.textPrimary
                            font.pixelSize: Theme.fontBody
                            font.weight: Font.DemiBold
                            elide: Text.ElideRight
                        }

                        Text {
                            Layout.fillWidth: true
                            text: deviceRow.modelData.address
                            color: Theme.textSecondary
                            font.pixelSize: Theme.fontSmall
                            elide: Text.ElideRight
                        }
                    }

                    Text {
                        text: deviceRow.isCurrent
                              ? (root.connected ? qsTr("Connected") : qsTr("Connecting…"))
                              : qsTr("Connect")
                        color: Theme.accent
                        font.pixelSize: Theme.fontBody
                        font.weight: Font.DemiBold
                    }
                }

                // MouseArea rather than TapHandler: it accepts the press, so the
                // tap does not also reach the map under the dialog
                MouseArea {
                    id: deviceArea
                    anchors.fill: parent
                    enabled: !deviceRow.isCurrent
                    onClicked: root.bluetooth.connectToDevice(deviceRow.modelData.address)
                }
            }

            // Empty list
            Text {
                visible: deviceList.count === 0
                anchors.centerIn: parent
                width: parent.width - 40
                horizontalAlignment: Text.AlignHCenter
                wrapMode: Text.WordWrap
                text: root.bluetooth.discovering
                      ? qsTr("Looking for nearby devices…")
                      : qsTr("No devices found. Turn on Bluetooth on the phone, pair it with this device and search again.")
                color: Theme.textSecondary
                font.pixelSize: Theme.fontBody
            }
        }
    }
}
