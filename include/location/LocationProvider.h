#pragma once

#include <QObject>

#include "data/Position.h"

class LocationProvider : public QObject {
    Q_OBJECT

    Q_PROPERTY(bool locationAvailable
               READ locationAvailable
               NOTIFY locationAvailableChanged)

    Q_PROPERTY(Position position
               READ position
               NOTIFY positionUpdated)

public:
    explicit LocationProvider(QObject *parent = nullptr);

    Q_INVOKABLE bool locationAvailable() const;
    Q_INVOKABLE Position position() const;

signals:
    void positionUpdated(const Position &position);
    void locationAvailableChanged(bool available);

private:
    bool m_locationAvailable = false;
    Position m_position;
};