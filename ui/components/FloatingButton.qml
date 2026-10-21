import QtQuick
import QtQuick.Effects
import QtQuick.Templates as T
import navigation

// Round white button with a shadow that floats over the map
T.Button {
    id: control

    property string iconPath
    property color iconColor: Theme.textSecondary

    implicitWidth: Theme.touchTarget
    implicitHeight: Theme.touchTarget

    scale: pressed ? 0.94 : 1
    Behavior on scale { NumberAnimation { duration: 80 } }

    contentItem: Item {
        Icon {
            anchors.centerIn: parent
            path: control.iconPath
            size: 26
            color: control.enabled ? control.iconColor : Theme.disabled
        }
    }

    background: Rectangle {
        radius: width / 2
        color: control.pressed ? Theme.surfacePressed : Theme.surface

        layer.enabled: true
        layer.effect: MultiEffect {
            shadowEnabled: true
            shadowColor: "#000000"
            shadowOpacity: 0.3
            shadowBlur: 0.5
            shadowVerticalOffset: 2
        }
    }
}
