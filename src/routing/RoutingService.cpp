#include "routing/RoutingService.h"

#include <cmath>

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>

RoutingService::RoutingService(const QUrl &serverUrl, QObject *parent)
    : QObject(parent),
      m_network(new QNetworkAccessManager(this)),
      m_serverUrl(serverUrl) {
}

const Route &RoutingService::route() const {
    return m_route;
}

QGeoCoordinate RoutingService::destination() const {
    return m_destination;
}

QVariantList RoutingService::path() const {
    return m_path;
}

bool RoutingService::busy() const {
    return m_busy;
}

QString RoutingService::errorString() const {
    return m_errorString;
}

void RoutingService::calculateRoute(const QGeoCoordinate &from,
                                    const QGeoCoordinate &to,
                                    double heading) {
    if (!from.isValid() || !to.isValid()) {
        setErrorString(tr("Invalid start or destination"));
        return;
    }

    // Only the newest request matters
    abortPendingRequest();

    m_destination = to;

    QJsonObject start { { "lat", from.latitude() }, { "lon", from.longitude() } };
    if (heading >= 0.0) {
        start.insert("heading", qRound(heading) % 360);
        start.insert("heading_tolerance", 45);
    }

    const QJsonObject body {
        { "locations", QJsonArray {
            start,
            QJsonObject { { "lat", to.latitude() }, { "lon", to.longitude() } },
        } },
        { "costing", "auto" },
        { "units", "kilometers" },
    };

    QUrl url = m_serverUrl;
    url.setPath("/route");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    setErrorString({});
    setBusy(true);

    QNetworkReply *reply = m_network->post(
        request, QJsonDocument(body).toJson(QJsonDocument::Compact));
    m_pendingReply = reply;
    connect(reply, &QNetworkReply::finished,
            this, [this, reply] { onReplyFinished(reply); });
}

void RoutingService::clearRoute() {
    abortPendingRequest();
    setRoute({});
    setErrorString({});
}

void RoutingService::abortPendingRequest() {
    if (!m_pendingReply)
        return;
    m_pendingReply->disconnect(this);
    m_pendingReply->abort();
    m_pendingReply->deleteLater();
    m_pendingReply = nullptr;
    setBusy(false);
}

void RoutingService::onReplyFinished(QNetworkReply *reply) {
    reply->deleteLater();
    if (reply == m_pendingReply)
        m_pendingReply = nullptr;
    setBusy(false);

    const QByteArray data = reply->readAll();
    const QJsonObject json = QJsonDocument::fromJson(data).object();

    if (reply->error() != QNetworkReply::NoError) {
        // Valhalla describes routing failures (e.g. no route found) in "error"
        const QString valhallaError = json.value("error").toString();
        setErrorString(valhallaError.isEmpty() ? reply->errorString()
                                               : valhallaError);
        return;
    }

    const Route route = parseRoute(json.value("trip").toObject());
    if (!route.isValid()) {
        setErrorString(tr("Valhalla returned no route"));
        return;
    }

    setRoute(route);
}

void RoutingService::setRoute(const Route &route) {
    m_route = route;
    m_path.clear();
    m_path.reserve(route.shape.size());
    for (const QGeoCoordinate &coordinate : route.shape)
        m_path.append(QVariant::fromValue(coordinate));
    emit routeChanged();
}

// Joins all legs into one shape; maneuver shape indices are shifted so they
// refer to the joined shape.
Route RoutingService::parseRoute(const QJsonObject &trip) {
    Route route;
    const QJsonArray legs = trip.value("legs").toArray();
    for (const QJsonValue &legValue : legs) {
        const QJsonObject leg = legValue.toObject();
        const int offset = route.shape.size();
        route.shape.append(decodePolyline(leg.value("shape").toString().toLatin1()));

        const QJsonArray maneuvers = leg.value("maneuvers").toArray();
        for (const QJsonValue &maneuverValue : maneuvers) {
            const QJsonObject json = maneuverValue.toObject();
            Maneuver maneuver;
            maneuver.type = json.value("type").toInt();
            maneuver.instruction = json.value("instruction").toString();
            maneuver.beginShapeIndex = offset + json.value("begin_shape_index").toInt();
            maneuver.endShapeIndex = offset + json.value("end_shape_index").toInt();
            maneuver.length = json.value("length").toDouble() * 1000.0;
            maneuver.time = json.value("time").toDouble();
            route.maneuvers.append(maneuver);
        }
    }

    const QJsonObject summary = trip.value("summary").toObject();
    route.length = summary.value("length").toDouble() * 1000.0;
    route.time = summary.value("time").toDouble();
    return route;
}

void RoutingService::setBusy(bool busy) {
    if (m_busy == busy)
        return;
    m_busy = busy;
    emit busyChanged();
}

void RoutingService::setErrorString(const QString &error) {
    if (m_errorString == error)
        return;
    m_errorString = error;
    emit errorStringChanged();
}

// Google encoded polyline format; Valhalla uses 6 decimal digits of precision
QList<QGeoCoordinate> RoutingService::decodePolyline(const QByteArray &encoded,
                                                     int precision) {
    QList<QGeoCoordinate> coordinates;
    const double factor = std::pow(10.0, precision);
    qsizetype index = 0;
    qint64 lat = 0;
    qint64 lon = 0;

    auto nextValue = [&](bool &ok) -> qint64 {
        qint64 result = 0;
        int shift = 0;
        int byte = 0;
        do {
            if (index >= encoded.size()) {
                ok = false;
                return 0;
            }
            byte = encoded.at(index++) - 63;
            result |= qint64(byte & 0x1f) << shift;
            shift += 5;
        } while (byte >= 0x20);
        return (result & 1) ? ~(result >> 1) : (result >> 1);
    };

    while (index < encoded.size()) {
        bool ok = true;
        const qint64 dLat = nextValue(ok);
        const qint64 dLon = nextValue(ok);
        if (!ok)
            break;
        lat += dLat;
        lon += dLon;
        coordinates.append(QGeoCoordinate(lat / factor, lon / factor));
    }

    return coordinates;
}
