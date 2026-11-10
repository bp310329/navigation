#pragma once

#include <QByteArray>
#include <QList>
#include <optional>

#include "Destination.h"

class QJsonValue;

// Parses destinations sent from the phone as JSON.
//
// One destination:
//   { "name": "Spodek", "latitude": 50.2661, "longitude": 19.0253 }
//
// A route with stops, in driving order; the last stop is the final destination:
//   { "stops": [ { "name": "...", "latitude": ..., "longitude": ... }, ... ] }
//
// "name" is optional everywhere.
class DestinationParser {
public:
    // Valhalla accepts 20 locations per request by default, one of them is the start
    static constexpr int MaxStops = 19;

    static std::optional<Destination> parse(const QByteArray &data);

    // Accepts a route with stops or a single destination (as a one-stop route).
    // Fails when any stop is invalid or there are more than MaxStops.
    static std::optional<QList<Destination>> parseStops(const QByteArray &data);

private:
    static std::optional<Destination> parseDestination(const QJsonValue &value);
};
