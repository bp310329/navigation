pragma Singleton
import QtQuick

// Colors and sizes shared by the whole UI. Sizes target a 7" 800x480
// touchscreen: touch targets are at least 56 px.
QtObject {
    // Brand
    readonly property color accent: "#1a73e8"
    readonly property color accentPressed: "#1557b0"
    readonly property color accentSoft: "#e8f0fe"
    readonly property color accentOnDark: "#8ab4f8"

    // Status
    readonly property color danger: "#d93025"
    readonly property color dangerSoft: "#fce8e6"
    readonly property color success: "#34a853"
    readonly property color warning: "#fbbc04"

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
    readonly property int statusBarHeight: 36
    readonly property int touchTarget: 56
    readonly property int spacing: 12
    readonly property int cardRadius: 20

    // Typography
    readonly property int fontSmall: 14
    readonly property int fontBody: 16
    readonly property int fontButton: 18
    readonly property int fontTitle: 20
}
