
cmake_minimum_required(VERSION 3.22)

project(navigation VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Qt
find_package(Qt6 6.8 REQUIRED COMPONENTS
    Quick
    Location
    Positioning
    Network
)

# MapLibre Native for Qt
find_package(QMapLibre REQUIRED COMPONENTS Location)

qt_standard_project_setup(REQUIRES 6.8)

qt_add_executable(appnavigation
    src/main.cpp
    src/controller/NavigationController.cpp
    src/bluetooth/BluetoothManager.cpp
    src/data/DestinationParser.cpp
    src/location/LocationProvider.cpp
    src/location/DummyPositionSource.cpp
    src/location/KeyboardDriver.cpp
    src/navigation/RouteTracker.cpp
    src/routing/RoutingService.cpp

    include/bluetooth/BluetoothManager.h
    include/controller/NavigationController.h
    include/data/Destination.h
    include/data/DestinationParser.h
    include/data/Position.h
    include/location/LocationProvider.h
    include/location/DummyPositionSource.h
    include/location/KeyboardDriver.h
    include/navigation/RouteTracker.h
    include/routing/Route.h
    include/routing/RoutingService.h
)

set_source_files_properties(ui/Theme.qml ui/Icons.qml
    PROPERTIES QT_QML_SINGLETON_TYPE TRUE
)

qt_add_qml_module(appnavigation
    URI navigation
    VERSION 1.0
    DEPENDENCIES
        QtPositioning
    QML_FILES
        ui/Main.qml
        ui/Theme.qml
        ui/Icons.qml
        ui/components/BatteryIndicator.qml
        ui/components/Card.qml
        ui/components/FloatingButton.qml
        ui/components/Icon.qml
        ui/components/ManeuverBanner.qml
        ui/components/MapView.qml
        ui/components/PillButton.qml
        ui/components/RoutePanel.qml
        ui/components/StatusBar.qml
        ui/components/TripSummary.qml
    RESOURCES
        assets/mapExample.png
)

set_target_properties(appnavigation PROPERTIES
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING
        ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

target_link_libraries(appnavigation
    PRIVATE
        Qt6::Quick
        Qt6::Location
        Qt6::Positioning
        Qt6::Network
        QMapLibre::Location
)

# Configure MapLibre QML plugins for the application
qmaplibre_location_setup_plugins(appnavigation)

if(WIN32)
    # A debug Qt app refuses to load release plugins ("Cannot mix debug and
    # release libraries"), so the "maplibre" geoservices provider would be missing.
    get_target_property(_maplibreConfigs QMapLibre::PluginGeoServices IMPORTED_CONFIGURATIONS)
    if(CMAKE_BUILD_TYPE STREQUAL "Debug" AND NOT "DEBUG" IN_LIST _maplibreConfigs)
        message(WARNING
            "QMapLibre was built without a Debug configuration. "
            "The map will not render in a Debug build; use Release or RelWithDebInfo.")
    endif()

    # Copy the MapLibre DLLs and the geoservices plugin next to the executable,
    # where Qt looks for them at runtime.
    add_custom_command(TARGET appnavigation POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:QMapLibre::Core>
            $<TARGET_FILE:QMapLibre::Location>
            $<TARGET_FILE:QMapLibre::QuickPrivate>
            $<TARGET_FILE_DIR:appnavigation>
        COMMAND ${CMAKE_COMMAND} -E make_directory
            $<TARGET_FILE_DIR:appnavigation>/geoservices
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:QMapLibre::PluginGeoServices>
            $<TARGET_FILE_DIR:appnavigation>/geoservices
    )
endif()

target_include_directories(appnavigation PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
    # Generated QML type registration includes QML_ELEMENT headers by file name
    "${CMAKE_CURRENT_SOURCE_DIR}/include/controller"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/data"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/location"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/routing"
)

include(GNUInstallDirs)

install(TARGETS appnavigation
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
