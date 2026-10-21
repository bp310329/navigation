import QtQuick
import QtQuick.Templates as T
import navigation

// Large rounded button. Primary is filled with the accent color,
// secondary is white with an outline.
T.Button {
    id: control

    property string iconPath
    property bool primary: true

    readonly property color foreground: !enabled ? Theme.surface
                                      : primary ? Theme.textOnAccent
                                                : Theme.textPrimary

    implicitHeight: Theme.touchTarget
    implicitWidth: Math.max(implicitHeight,
                            implicitContentWidth + leftPadding + rightPadding)
    leftPadding: iconPath && !text ? 0 : 22
    rightPadding: leftPadding
    spacing: 10

    font.pixelSize: Theme.fontButton
    font.weight: Font.DemiBold

    scale: pressed ? 0.97 : 1
    Behavior on scale { NumberAnimation { duration: 80 } }

    contentItem: Item {
        implicitWidth: row.implicitWidth
        implicitHeight: row.implicitHeight

        Row {
            id: row
            anchors.centerIn: parent
            spacing: control.spacing

            Icon {
                visible: control.iconPath.length > 0
                anchors.verticalCenter: parent.verticalCenter
                path: control.iconPath
                size: 24
                color: control.foreground
            }

            Text {
                visible: text.length > 0
                anchors.verticalCenter: parent.verticalCenter
                text: control.text
                font: control.font
                color: control.foreground
            }
        }
    }

    background: Rectangle {
        radius: height / 2
        color: !control.enabled ? Theme.disabled
             : control.primary ? (control.pressed ? Theme.accentPressed : Theme.accent)
                               : (control.pressed ? Theme.surfacePressed : Theme.surface)
        border.width: control.primary ? 0 : 1
        border.color: Theme.outline
    }
}
