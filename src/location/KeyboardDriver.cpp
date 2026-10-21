#include "location/KeyboardDriver.h"

#include <algorithm>
#include <cmath>

#include <QKeyEvent>
#include <QWindow>

#include "location/DummyPositionSource.h"

namespace {
constexpr int TickMs = 50;

constexpr double CruiseSpeed = 50.0 / 3.6;   // m/s with W
constexpr double BoostSpeed = 120.0 / 3.6;   // m/s with Shift + W
constexpr double ReverseSpeed = 15.0 / 3.6;  // m/s with S
constexpr double Acceleration = 6.0;         // m/s^2
constexpr double Deceleration = 10.0;        // m/s^2, braking and rolling to a stop
constexpr double TurnRate = 90.0;            // degrees per second at full steering

// Moves `value` toward `target` by at most `maxStep`
double approach(double value, double target, double maxStep) {
    if (value < target)
        return std::min(value + maxStep, target);
    return std::max(value - maxStep, target);
}
}

KeyboardDriver::KeyboardDriver(DummyPositionSource *source, QObject *parent)
    : QObject(parent),
      m_source(source) {
    m_timer.setInterval(TickMs);
    connect(&m_timer, &QTimer::timeout, this, &KeyboardDriver::step);
}

void KeyboardDriver::setHeading(double degrees) {
    m_heading = degrees;
    m_source->setDirection(m_heading);
}

bool KeyboardDriver::eventFilter(QObject *watched, QEvent *event) {
    // Key events are also delivered to each Quick item; only count the window's copy
    const bool isKey = event->type() == QEvent::KeyPress
                    || event->type() == QEvent::KeyRelease;
    if (!isKey || !qobject_cast<QWindow *>(watched))
        return false;

    auto *keyEvent = static_cast<QKeyEvent *>(event);
    if (keyEvent->isAutoRepeat())
        return false;

    const bool pressed = event->type() == QEvent::KeyPress;
    switch (keyEvent->key()) {
    case Qt::Key_W: case Qt::Key_Up:    m_forward = pressed; break;
    case Qt::Key_S: case Qt::Key_Down:  m_backward = pressed; break;
    case Qt::Key_A: case Qt::Key_Left:  m_left = pressed; break;
    case Qt::Key_D: case Qt::Key_Right: m_right = pressed; break;
    case Qt::Key_Shift:                 m_boost = pressed; break;
    default: return false;
    }

    if (!m_timer.isActive() && anyKeyHeld()) {
        m_clock.start();
        m_timer.start();
    }
    return false;
}

void KeyboardDriver::step() {
    const double dt = m_clock.restart() / 1000.0;

    // Throttle and brake
    double targetSpeed = 0.0;
    if (m_forward && !m_backward)
        targetSpeed = m_boost ? BoostSpeed : CruiseSpeed;
    else if (m_backward && !m_forward)
        targetSpeed = m_speed > 0.5 ? 0.0 : -ReverseSpeed; // brake before reversing
    const bool speedingUp = std::abs(targetSpeed) > std::abs(m_speed);
    m_speed = approach(m_speed, targetSpeed, (speedingUp ? Acceleration : Deceleration) * dt);

    // Steering; also works standing still so the start direction can be set up
    const int steer = (m_right ? 1 : 0) - (m_left ? 1 : 0);
    const double direction = m_speed < 0.0 ? -1.0 : 1.0; // steering inverts when reversing
    m_heading = std::fmod(m_heading + steer * direction * TurnRate * dt + 360.0, 360.0);

    // Move
    if (m_speed != 0.0) {
        const double distance = std::abs(m_speed) * dt;
        const double azimuth = m_speed > 0.0 ? m_heading : std::fmod(m_heading + 180.0, 360.0);
        m_source->setCoordinate(m_source->coordinate().atDistanceAndAzimuth(distance, azimuth));
    }

    m_source->setDirection(m_heading);
    m_source->setGroundSpeed(std::abs(m_speed));
    m_source->requestUpdate();

    if (!anyKeyHeld() && m_speed == 0.0)
        m_timer.stop();
}

bool KeyboardDriver::anyKeyHeld() const {
    return m_forward || m_backward || m_left || m_right;
}
