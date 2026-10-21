#pragma once

#include <QElapsedTimer>
#include <QObject>
#include <QTimer>

class DummyPositionSource;

// Testing aid: "drives" a DummyPositionSource with the keyboard.
//   W / Up     accelerate forward      S / Down    brake, then reverse
//   A / Left   steer left              D / Right   steer right
//   Shift      faster top speed
// Install as an application event filter; key events are observed, not consumed.
class KeyboardDriver : public QObject {
    Q_OBJECT

public:
    explicit KeyboardDriver(DummyPositionSource *source, QObject *parent = nullptr);

    // Points the simulated car in a direction (degrees), e.g. along the route
    void setHeading(double degrees);

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    void step();
    bool anyKeyHeld() const;

    DummyPositionSource *m_source;
    QTimer m_timer;
    QElapsedTimer m_clock;

    bool m_forward = false;
    bool m_backward = false;
    bool m_left = false;
    bool m_right = false;
    bool m_boost = false;

    double m_heading = 0.0; // degrees clockwise from north
    double m_speed = 0.0;   // m/s, negative when reversing
};
