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

// Calculates routes with a Valhalla server (POST /route). The full route with
// maneuvers is available to C++ via route(); QML gets the geometry as `path`,
// a list of coordinates ready for MapPolyline.path.
class RoutingService : public QObject {
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Provided by the application")

    Q_PROPERTY(QVariantList path READ path NOTIFY routeChanged)
    Q_PROPERTY(bool busy READ busy NOTIFY busyChanged)
    Q_PROPERTY(QString errorString READ errorString NOTIFY errorStringChanged)

public:
    explicit RoutingService(const QUrl &serverUrl, QObject *parent = nullptr);

    const Route &route() const;
    QVariantList path() const;
    bool busy() const;
    QString errorString() const;

    Q_INVOKABLE void calculateRoute(const QGeoCoordinate &from,
                                    const QGeoCoordinate &to);
    Q_INVOKABLE void clearRoute();

signals:
    void routeChanged();
    void busyChanged();
    void errorStringChanged();

private:
    void abortPendingRequest();
    void onReplyFinished(QNetworkReply *reply);
    void setRoute(const Route &route);
    void setBusy(bool busy);
    void setErrorString(const QString &error);

    static Route parseRoute(const QJsonObject &trip);
    static QList<QGeoCoordinate> decodePolyline(const QByteArray &encoded,
                                                int precision = 6);

    QNetworkAccessManager *m_network;
    QUrl m_serverUrl;
    QNetworkReply *m_pendingReply = nullptr;
    Route m_route;
    QVariantList m_path;
    bool m_busy = false;
    QString m_errorString;
};
