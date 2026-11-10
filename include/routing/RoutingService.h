#pragma once

#include <QObject>
#include <QUrl>
#include <QVariantList>
#include <QGeoCoordinate>
#include <QtQml/qqmlregistration.h>

#include "routing/Route.h"

class QJsonObject;
class QNetworkAccessManager;
class QNetworkReply;

// Calculates routes with a Valhalla server (POST /route). A route goes from
// the start through one or more stops in order. The full route with maneuvers
// is available to C++ via route(); QML gets the geometry as `path`, a list of
// coordinates ready for MapPolyline.path, and the stops as `stops`.
class RoutingService : public QObject {
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Provided by the application")

    Q_PROPERTY(QVariantList path READ path NOTIFY routeChanged)
    // Stops of the current route: [{ name, coordinate, reached: false }]
    Q_PROPERTY(QVariantList stops READ stops NOTIFY routeChanged)
    Q_PROPERTY(bool busy READ busy NOTIFY busyChanged)
    Q_PROPERTY(QString errorString READ errorString NOTIFY errorStringChanged)

public:
    explicit RoutingService(const QUrl &serverUrl, QObject *parent = nullptr);

    const Route &route() const;
    QVariantList path() const;
    QVariantList stops() const;
    bool busy() const;
    QString errorString() const;

    // Route from `from` through `stops` in order, ending at the last one.
    // `heading` (degrees, optional) is the current direction of travel; it
    // keeps the route from starting with a turn back the way we came.
    void calculateRoute(const QGeoCoordinate &from,
                        const QList<Destination> &stops,
                        double heading = -1.0);

    // Single unnamed destination, e.g. picked on the map
    Q_INVOKABLE void calculateRoute(const QGeoCoordinate &from,
                                    const QGeoCoordinate &to,
                                    double heading = -1.0);
    Q_INVOKABLE void clearRoute();

    // Stops as a QML list: [{ name, coordinate, reached }], where the first
    // `reachedCount` stops are marked as reached
    static QVariantList stopsToVariantList(const QList<Destination> &stops,
                                           int reachedCount = 0);

signals:
    void routeChanged();
    void busyChanged();
    void errorStringChanged();

private:
    void abortPendingRequest();
    void onReplyFinished(QNetworkReply *reply, const QList<Destination> &stops);
    void setRoute(const Route &route);
    void setBusy(bool busy);
    void setErrorString(const QString &error);

    static Route parseRoute(const QJsonObject &trip, const QList<Destination> &stops);
    static QList<QGeoCoordinate> decodePolyline(const QByteArray &encoded,
                                                int precision = 6);

    QNetworkAccessManager *m_network;
    QUrl m_serverUrl;
    QNetworkReply *m_pendingReply = nullptr;
    Route m_route;
    QVariantList m_path;
    QVariantList m_stops;
    bool m_busy = false;
    QString m_errorString;
};
