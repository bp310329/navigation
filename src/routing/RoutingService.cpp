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
                                    const QGeoCoordinate &to) {
    if (!from.isValid() || !to.isValid()) {
        setErrorString(tr("Invalid start or destination"));
        return;
    }

    // Only the newest request matters
    abortPendingRequest();

    const QJsonObject body {
        { "locations", QJsonArray {
            QJsonObject { { "lat", from.latitude() }, { "lon", from.longitude() } },
            QJsonObject { { "lat", to.latitude() }, { "lon", to.longitude() } },
        } },
        { "costing", "auto" },
        // Turn-by-turn instructions are not needed yet, only the geometry
        { "directions_type", "none" },
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
    setPath({});
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

    QVariantList path;
    const QJsonArray legs = json.value("trip").toObject().value("legs").toArray();
    for (const QJsonValue &leg : legs) {
        const QByteArray shape = leg.toObject().value("shape").toString().toLatin1();
        for (const QGeoCoordinate &coordinate : decodePolyline(shape))
            path.append(QVariant::fromValue(coordinate));
    }

    if (path.isEmpty()) {
        setErrorString(tr("Valhalla returned no route"));
        return;
    }

    setPath(path);
}

void RoutingService::setPath(const QVariantList &path) {
    m_path = path;
    emit pathChanged();
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
