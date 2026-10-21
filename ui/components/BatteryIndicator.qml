import QtQuick
import navigation

// Horizontal battery glyph filled according to `level` (0-100)
Item {
    id: root

    property int level: 100
    property color color: Theme.textOnDark

    readonly property color fillColor: level <= 15 ? Theme.danger
                                     : level <= 30 ? Theme.warning
                                                   : Theme.success

    implicitWidth: 40
    implicitHeight: 20

    Rectangle {
        id: body
        width: parent.width - 4
        height: parent.height
        radius: 4
        color: "transparent"
        border.color: root.color
        border.width: 2

        Rectangle {
            x: 4
            anchors.verticalCenter: parent.verticalCenter
            width: Math.max(0, (parent.width - 8) * Math.min(root.level, 100) / 100)
            height: parent.height - 8
            radius: 2
            color: root.fillColor
        }
    }

    // Terminal nub
    Rectangle {
        anchors.left: body.right
        anchors.verticalCenter: body.verticalCenter
        width: 3
        height: 8
        radius: 1.5
        color: root.color
    }
}
