import QtQuick
import QtQuick.Effects
import navigation

// Rounded surface with a soft shadow, floating over the map
Rectangle {
    radius: Theme.cardRadius
    color: Theme.surface

    layer.enabled: true
    layer.effect: MultiEffect {
        shadowEnabled: true
        shadowColor: "#000000"
        shadowOpacity: 0.25
        shadowBlur: 0.8
        shadowVerticalOffset: 3
    }
}
