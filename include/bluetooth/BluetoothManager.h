#pragma once

#include <QObject>
#include <QBluetoothSocket>
#include <QBluetoothAddress>
#include <QBluetoothUuid>


class BluetoothManager : public QObject {
    Q_OBJECT

public:
    explicit BluetoothManager(QObject *parent = nullptr);

    void connectToDevice(const QBluetoothAddress &address, const QBluetoothUuid &uuid);
    void sendData(const QByteArray &data);

signals:
    void connected();
    void disconnected();
    void dataReceived(const QByteArray &data);
    void errorOccurred(const QString &error);

private:
    QBluetoothSocket *socket;
    QByteArray receiveBuffer;
};