Software of navigation device based on Raspberry Pi 4. Developing a C++ application using Qt Quick/QML, integrating MapLibre Native Qt for map rendering and Valhalla for route calculation based on OpenStreetMap data. 
The project also involves working with Linux, CMake, native ARM64 library compilation, and hardware-software integration. Ultimately this projects will consists of this device and mobile app to create routes. [WIP]

## Building

Requires Qt 6.8+ (Quick, Location, Positioning, Network), MapLibre Native Qt and Ninja.

Each device keeps its own paths in `CMakeUserPresets.json`, which is not committed:

1. Copy `CMakeUserPresets.json.example` to `CMakeUserPresets.json`.
2. Set `inherits` to `windows` or `linux`.
3. Set `CMAKE_PREFIX_PATH` to this device's Qt and MapLibre install directories.
4. Set `NAV_VALHALLA_URL` to this device's Valhalla server.

Then run:

```sh
cmake --preset local
cmake --build --preset local
```

The output goes to `build/local/`. In VS Code, CMake Tools picks up the `local` preset automatically.
To use a different Valhalla server without rebuilding, set the `NAV_VALHALLA_URL` environment variable before starting the app.
