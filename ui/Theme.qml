pragma Singleton
import QtQuick

// Colors and sizes shared by the whole UI. Sizes target a 7" 1280x720
// touchscreen (~210 ppi): touch targets are at least 80 px (~10 mm).
QtObject {
    // Brand
    readonly property color accent: "#1a73e8"
    readonly property color accentPressed: "#1557b0"
    readonly property color accentSoft: "#e8f0fe"
    readonly property color accentOnDark: "#8ab4f8"

    // Status
    readonly property color danger: "#d93025"
    readonly property color dangerPressed: "#b3261e"
    readonly property color dangerSoft: "#fce8e6"
    readonly property color success: "#34a853"
    readonly property color warning: "#fbbc04"

    // Navigation
    readonly property color maneuver: "#0d652d"
    readonly property color maneuverSecondary: "#0a5224"
    readonly property color eta: "#188038"

    // Surfaces
    readonly property color chrome: "#15181d"
    readonly property color surface: "#ffffff"
    readonly property color surfacePressed: "#eceff3"
    readonly property color outline: "#dadce0"
    readonly property color disabled: "#bdc1c6"

    // Text
    readonly property color textPrimary: "#202124"
    readonly property color textSecondary: "#5f6368"
    readonly property color textOnAccent: "#ffffff"
    readonly property color textOnDark: "#e8eaed"
    readonly property color textOnDarkMuted: "#9aa0a6"

    // Metrics
    readonly property int statusBarHeight: 52
    readonly property int touchTarget: 80
    readonly property int spacing: 18
    readonly property int cardPadding: 24
    readonly property int cardRadius: 28

    // Icons
    readonly property int iconSmall: 28
    readonly property int iconMedium: 34
    readonly property int iconLarge: 40
    readonly property int iconManeuver: 76

    // Typography
    readonly property int fontSmall: 20
    readonly property int fontBody: 22
    readonly property int fontButton: 26
    readonly property int fontTitle: 30
    readonly property int fontHeadline: 36
    readonly property int fontDisplay: 48
}
