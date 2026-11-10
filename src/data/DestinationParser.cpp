#include "data/DestinationParser.h"
#include "data/Destination.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <cmath>

namespace {
std::optional<QJsonObject> parseObject(const QByteArray &data) {
    QJsonParseError error;
    const QJsonDocument document = QJsonDocument::fromJson(data, &error);
    if (error.error != QJsonParseError::NoError || !document.isObject())
        return std::nullopt;
    return document.object();
}
}

std::optional<Destination> DestinationParser::parse(const QByteArray &data) {
    const std::optional<QJsonObject> object = parseObject(data);
    if (!object)
        return std::nullopt;
    return parseDestination(*object);
}

std::optional<QList<Destination>> DestinationParser::parseStops(const QByteArray &data) {
    const std::optional<QJsonObject> object = parseObject(data);
    if (!object)
        return std::nullopt;

    if (!object->contains("stops")) {
        const std::optional<Destination> destination = parseDestination(*object);
        if (!destination)
            return std::nullopt;
        return QList<Destination> { *destination };
    }

    const QJsonValue stopsValue = object->value("stops");
    if (!stopsValue.isArray())
        return std::nullopt;
    const QJsonArray stopsArray = stopsValue.toArray();
    if (stopsArray.isEmpty() || stopsArray.size() > MaxStops)
        return std::nullopt;

    QList<Destination> stops;
    stops.reserve(stopsArray.size());
    for (const QJsonValue &value : stopsArray) {
        const std::optional<Destination> stop = parseDestination(value);
        if (!stop)
            return std::nullopt;
        stops.append(*stop);
    }
    return stops;
}

std::optional<Destination> DestinationParser::parseDestination(const QJsonValue &value) {
    if (!value.isObject())
        return std::nullopt;
    const QJsonObject object = value.toObject();

    const QJsonValue lat = object.value("latitude");
    const QJsonValue lon = object.value("longitude");

    if (!lat.isDouble() || !lon.isDouble()) {
        return std::nullopt;
    }

    const double latitude = lat.toDouble();
    const double longitude = lon.toDouble();

    if (!std::isfinite(latitude) ||
        !std::isfinite(longitude)) {
        return std::nullopt;
    }

    if (latitude < -90.0 || latitude > 90.0 ||
        longitude < -180.0 || longitude > 180.0) {
        return std::nullopt;
    }

    Destination destination;
    destination.latitude = latitude;
    destination.longitude = longitude;
    destination.name = object.value("name").toString();

    return destination;
}
