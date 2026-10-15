import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ApplicationWindow {
    width: 1280
    height: 720
    visible: true
    title: qsTr("Navigation")

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 20
        spacing: 20

        Label {
            text: "NO CONNECTION"
            font.pixelSize: 16
        }

        MapView {
            Layout.fillWidth: true
            Layout.fillHeight: true
        }

        Button {
            text: "Start Navigation"
            onClicked: {
                controller.startNavigation()
            }
        }
    }
}
