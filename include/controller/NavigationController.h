#pragma once

#include <QObject>

#include "data/Destination.h"

class LocationProvider;

class NavigationController : public QObject {
    Q_OBJECT

    Q_PROPERTY(bool navigating
               READ navigating
               NOTIFY navigatingChanged)

public:
    NavigationController(
        LocationProvider *locationProvider,
        QObject *parent = nullptr
    );

    bool navigating() const;

    Q_INVOKABLE void startNavigation(const Destination &destination);
    Q_INVOKABLE void stopNavigation();

signals:
    void navigatingChanged(bool navigating);

private:
    LocationProvider *m_locationProvider;
    bool m_navigating = false;
};