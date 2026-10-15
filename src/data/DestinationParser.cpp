#include "data/DestinationParser.h"
#include "data/Destination.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <cmath>

std::optional<Destination> DestinationParser::parse(const QByteArray &data) {
    QJsonParseError error;

    const QJsonDocument document =
        QJsonDocument::fromJson(data, &error);

    if (error.error != QJsonParseError::NoError ||
        !document.isObject()) {
        return std::nullopt;
    }

    const QJsonObject object = document.object();

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