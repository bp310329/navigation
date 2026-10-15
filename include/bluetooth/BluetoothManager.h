#pragma once

#include <QObject>

class BluetoothManager : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool bluetoothStatus
               READ bluetoothStatus
               NOTIFY bluetoothStatusChanged)

public:
    explicit BluetoothManager(QObject *parent = nullptr) : QObject(parent) {};

    Q_INVOKABLE void startBluetooth();
    Q_INVOKABLE void stopBluetooth();

    bool bluetoothStatus() const;

signals:
    void bluetoothStatusChanged(bool status);

private:
    bool m_bluetoothStatus = false;
};