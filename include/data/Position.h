#pragma once

#include <QObject>

struct Position {
    double latitude = 0.0;
    double longitude = 0.0;
    double speed = 0.0;
    bool valid = false;
};