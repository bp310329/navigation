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

    implicitWidth: 28
    implicitHeight: 14

    Rectangle {
        id: body
        width: parent.width - 3
        height: parent.height
        radius: 3
        color: "transparent"
        border.color: root.color
        border.width: 1.5

        Rectangle {
            x: 3
            anchors.verticalCenter: parent.verticalCenter
            width: Math.max(0, (parent.width - 6) * Math.min(root.level, 100) / 100)
            height: parent.height - 6
            radius: 1.5
            color: root.fillColor
        }
    }

    // Terminal nub
    Rectangle {
        anchors.left: body.right
        anchors.verticalCenter: body.verticalCenter
        width: 2
        height: 6
        radius: 1
        color: root.color
    }
}
