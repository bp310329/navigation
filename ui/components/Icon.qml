import QtQuick
import QtQuick.Shapes
import navigation

// Vector icon from a 24x24 SVG path (see Icons.qml), scaled to `size`
Item {
    id: root

    property string path
    property real size: 24
    property color color: Theme.textPrimary
    property color strokeColor: "transparent"
    property real strokeWidth: 0 // in 24x24 icon units

    implicitWidth: size
    implicitHeight: size

    Shape {
        width: 24
        height: 24
        anchors.centerIn: parent
        scale: root.size / 24
        preferredRendererType: Shape.CurveRenderer

        ShapePath {
            fillColor: root.color
            fillRule: ShapePath.WindingFill
            strokeColor: root.strokeColor
            strokeWidth: root.strokeWidth
            joinStyle: ShapePath.RoundJoin

            PathSvg { path: root.path }
        }
    }
}
