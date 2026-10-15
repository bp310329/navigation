#pragma once

#include <QObject>
#include <QGeoCoordinate>
#include <QtQml/qqmlregistration.h>

struct Position {
    Q_GADGET
    QML_VALUE_TYPE(position)

    Q_PROPERTY(double latitude MEMBER latitude)
    Q_PROPERTY(double longitude MEMBER longitude)
    Q_PROPERTY(double speed MEMBER speed)
    Q_PROPERTY(bool valid MEMBER valid)
    Q_PROPERTY(QGeoCoordinate coordinate READ coordinate)

public:
    double latitude = 0.0;
    double longitude = 0.0;
    double speed = 0.0;
    bool valid = false;

    QGeoCoordinate coordinate() const {
        return valid ? QGeoCoordinate(latitude, longitude) : QGeoCoordinate();
    }
};
