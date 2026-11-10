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

## Routes from the phone

### Bluetooth connection

Tap the Bluetooth status in the top-left corner. This opens a list of nearby devices; pick the phone to connect to it.

- The device connects to the phone over Bluetooth Classic (RFCOMM). The phone app must listen for connections on the Serial Port Profile UUID `00001101-0000-1000-8000-00805F9B34FB`.
- Pair the phone with the device in the system Bluetooth settings first. On Windows, the app can't start pairing itself.
- Messages in both directions are UTF-8 JSON, one per line, each ending with `
`.
- The device answers each message with an acknowledgement: `{"type":"ack","ok":true}`, or `{"type":"ack","ok":false,"error":"invalid route"}` if it can't read the route.

### Route format

The phone sends a route as JSON. The stops are in driving order, and the last one is the final destination. `name` is optional. A route can have at most 19 stops.

```json
{
    "stops": [
        { "name": "Spodek", "latitude": 50.2662, "longitude": 19.0253 },
        { "name": "Chorzów Rynek", "latitude": 50.2977, "longitude": 18.9543 }
    ]
}
```

A single destination object (`{ "name": ..., "latitude": ..., "longitude": ... }`) is also accepted, as a route with one stop.

`TripReceiver::receive()` takes this data and calculates the route from the current position. While navigating, stops are reached in order. If the car leaves the route, the new route goes only through the stops that haven't been reached yet.

To test without a phone, load a route from a file:

```sh
appnavigation --trip testdata/trip-katowice.json
```
