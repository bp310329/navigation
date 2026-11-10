#pragma once

#include <QBluetoothDeviceInfo>
#include <QBluetoothSocket>
#include <QBluetoothUuid>
#include <QList>
#include <QObject>
#include <QTimer>
#include <QVariantList>
#include <QtQml/qqmlregistration.h>

class QBluetoothDeviceDiscoveryAgent;

// Connection to the phone over Bluetooth Classic (RFCOMM). Finds nearby
// devices, connects to the phone's service and splits the incoming data
// into messages.
//
// Protocol: the phone listens on an RFCOMM service with ServiceUuid. Both
// sides send UTF-8 JSON messages, one per line, each terminated by '\n'.
class BluetoothManager : public QObject {
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Provided by the application")

    Q_PROPERTY(State state READ state NOTIFY stateChanged)
    // Name and address of the device connected or being connected to
    Q_PROPERTY(QString deviceName READ deviceName NOTIFY stateChanged)
    Q_PROPERTY(QString deviceAddress READ deviceAddress NOTIFY stateChanged)

    Q_PROPERTY(bool discovering READ discovering NOTIFY discoveringChanged)
    // Devices found by discovery: [{ name, address, isPhone }], phones first
    Q_PROPERTY(QVariantList devices READ devices NOTIFY devicesChanged)

    // Last discovery or connection error; cleared by the next attempt
    Q_PROPERTY(QString errorString READ errorString NOTIFY errorStringChanged)

public:
    enum State {
        Disconnected,
        Connecting,
        Connected,
    };
    Q_ENUM(State)

    // Serial Port Profile; the phone app has to offer its service under this UUID
    static const QBluetoothUuid ServiceUuid;

    explicit BluetoothManager(QObject *parent = nullptr);
    ~BluetoothManager() override;

    State state() const;
    QString deviceName() const;
    QString deviceAddress() const;
    bool discovering() const;
    QVariantList devices() const;
    QString errorString() const;

    Q_INVOKABLE void startDiscovery();
    Q_INVOKABLE void stopDiscovery();

    // `address` as in `devices`, e.g. "AA:BB:CC:DD:EE:FF"
    Q_INVOKABLE void connectToDevice(const QString &address);
    Q_INVOKABLE void disconnectFromDevice();

    // Sends one message; the line terminator is added here
    void sendMessage(const QByteArray &message);

signals:
    void stateChanged();
    void discoveringChanged();
    void devicesChanged();
    void errorStringChanged();
    // One complete message from the phone, without the line terminator
    void messageReceived(const QByteArray &message);

private:
    void onDeviceDiscovered(const QBluetoothDeviceInfo &info);
    void onDiscoveryFinished();
    void onSocketConnected();
    void onSocketDisconnected();
    void onSocketError(QBluetoothSocket::SocketError error);
    void onReadyRead();
    void closeSocket();
    void setState(State state);
    void setDiscovering(bool discovering);
    void setErrorString(const QString &error);

    QBluetoothDeviceDiscoveryAgent *m_discoveryAgent;
    QBluetoothSocket *m_socket = nullptr;
    QTimer m_connectTimeout;
    QList<QBluetoothDeviceInfo> m_devices;
    QByteArray m_receiveBuffer;

    State m_state = Disconnected;
    QString m_deviceName;
    QString m_deviceAddress;
    bool m_discovering = false;
    QString m_errorString;
};
