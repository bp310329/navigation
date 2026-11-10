// BluetoothManager.cpp

#include "bluetooth/BluetoothManager.h"

BluetoothManager::BluetoothManager(QObject *parent)
    : QObject(parent)
{
    socket = new QBluetoothSocket(
        QBluetoothServiceInfo::RfcommProtocol,
        this
    );

    connect(socket, &QBluetoothSocket::connected,
            this, &BluetoothManager::connected);

    connect(socket, &QBluetoothSocket::disconnected,
            this, &BluetoothManager::disconnected);

    connect(socket, &QBluetoothSocket::readyRead,
            this, [this]() {
        receiveBuffer.append(socket->readAll());

        // Tutaj docelowo nastąpi dekodowanie
        // ramek protokołu komunikacyjnego.
        emit dataReceived(receiveBuffer);
        receiveBuffer.clear();
    });

    connect(socket, &QBluetoothSocket::errorOccurred,
            this, [this](QBluetoothSocket::SocketError) {
        emit errorOccurred(socket->errorString());
    });
}

void BluetoothManager::connectToDevice(
    const QBluetoothAddress &address,
    const QBluetoothUuid &serviceUuid)
{
    socket->connectToService(
        address,
        serviceUuid,
        QIODevice::ReadWrite
    );
}

void BluetoothManager::sendData(const QByteArray &data)
{
    if (socket->isOpen()) {
        socket->write(data);
    }
}