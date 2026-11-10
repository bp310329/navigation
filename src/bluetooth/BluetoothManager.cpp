#include "bluetooth/BluetoothManager.h"

#include <QBluetoothAddress>
#include <QBluetoothDeviceDiscoveryAgent>
#include <QDebug>
#include <algorithm>

namespace {
// Classic discovery takes about 10 s; this ends it in any case
constexpr int DiscoveryTimeoutMs = 20000;
// Connecting to a device that is out of range can otherwise take very long
constexpr int ConnectTimeoutMs = 20000;
// A longer line without a terminator is not a valid message; it is dropped
constexpr qsizetype MaxMessageSize = 64 * 1024;
}

const QBluetoothUuid BluetoothManager::ServiceUuid =
    QBluetoothUuid(QBluetoothUuid::ServiceClassUuid::SerialPort);

BluetoothManager::BluetoothManager(QObject *parent)
    : QObject(parent),
      m_discoveryAgent(new QBluetoothDeviceDiscoveryAgent(this)) {
    m_discoveryAgent->setLowEnergyDiscoveryTimeout(DiscoveryTimeoutMs);

    connect(m_discoveryAgent, &QBluetoothDeviceDiscoveryAgent::deviceDiscovered,
            this, &BluetoothManager::onDeviceDiscovered);
    connect(m_discoveryAgent, &QBluetoothDeviceDiscoveryAgent::deviceUpdated,
            this, [this](const QBluetoothDeviceInfo &info) { onDeviceDiscovered(info); });
    connect(m_discoveryAgent, &QBluetoothDeviceDiscoveryAgent::finished,
            this, &BluetoothManager::onDiscoveryFinished);
    connect(m_discoveryAgent, &QBluetoothDeviceDiscoveryAgent::canceled,
            this, &BluetoothManager::onDiscoveryFinished);
    connect(m_discoveryAgent, &QBluetoothDeviceDiscoveryAgent::errorOccurred,
            this, [this](QBluetoothDeviceDiscoveryAgent::Error error) {
        switch (error) {
        case QBluetoothDeviceDiscoveryAgent::PoweredOffError:
            setErrorString(tr("Bluetooth is turned off"));
            break;
        case QBluetoothDeviceDiscoveryAgent::InvalidBluetoothAdapterError:
            setErrorString(tr("No Bluetooth adapter found"));
            break;
        default:
            setErrorString(m_discoveryAgent->errorString());
            break;
        }
        setDiscovering(false);
    });

    m_connectTimeout.setSingleShot(true);
    m_connectTimeout.setInterval(ConnectTimeoutMs);
    connect(&m_connectTimeout, &QTimer::timeout, this, [this] {
        if (m_state != Connecting)
            return;
        closeSocket();
        setErrorString(tr("Connection timed out"));
        setState(Disconnected);
    });
}

BluetoothManager::~BluetoothManager() {
    closeSocket();
}

BluetoothManager::State BluetoothManager::state() const { return m_state; }
QString BluetoothManager::deviceName() const { return m_deviceName; }
QString BluetoothManager::deviceAddress() const { return m_deviceAddress; }
bool BluetoothManager::discovering() const { return m_discovering; }
QString BluetoothManager::errorString() const { return m_errorString; }

QVariantList BluetoothManager::devices() const {
    QVariantList list;
    list.reserve(m_devices.size());
    for (const QBluetoothDeviceInfo &info : m_devices) {
        list.append(QVariantMap {
            { "name", info.name() },
            { "address", info.address().toString() },
            { "isPhone", info.majorDeviceClass() == QBluetoothDeviceInfo::PhoneDevice },
        });
    }
    return list;
}

void BluetoothManager::startDiscovery() {
    if (m_discovering)
        return;
    setErrorString({});
    m_devices.clear();
    emit devicesChanged();
    // Phones are Bluetooth Classic devices; RFCOMM does not work over LE
    m_discoveryAgent->start(QBluetoothDeviceDiscoveryAgent::ClassicMethod);
    setDiscovering(m_discoveryAgent->isActive());
}

void BluetoothManager::stopDiscovery() {
    if (m_discoveryAgent->isActive())
        m_discoveryAgent->stop();
    setDiscovering(false);
}

void BluetoothManager::onDeviceDiscovered(const QBluetoothDeviceInfo &info) {
    // Only Classic devices can offer an RFCOMM service
    if (!(info.coreConfigurations() & QBluetoothDeviceInfo::BaseRateCoreConfiguration))
        return;

    const auto isPhone = [](const QBluetoothDeviceInfo &device) {
        return device.majorDeviceClass() == QBluetoothDeviceInfo::PhoneDevice;
    };

    const auto existing = std::find_if(m_devices.begin(), m_devices.end(),
        [&](const QBluetoothDeviceInfo &device) { return device.address() == info.address(); });
    if (existing != m_devices.end()) {
        *existing = info;
    } else if (isPhone(info)) {
        // Phones first. Devices already listed keep their place, so a row
        // does not move away while the user is about to tap it.
        const auto firstOther = std::find_if_not(m_devices.begin(), m_devices.end(), isPhone);
        m_devices.insert(firstOther, info);
    } else {
        m_devices.append(info);
    }
    emit devicesChanged();
}

void BluetoothManager::onDiscoveryFinished() {
    setDiscovering(false);
}

void BluetoothManager::connectToDevice(const QString &address) {
    const QBluetoothAddress bluetoothAddress(address);
    if (bluetoothAddress.isNull()) {
        setErrorString(tr("Invalid device address"));
        return;
    }

    // Discovery slows down connecting and is not needed any more
    stopDiscovery();
    closeSocket();

    m_deviceAddress = bluetoothAddress.toString();
    m_deviceName.clear();
    for (const QBluetoothDeviceInfo &info : std::as_const(m_devices)) {
        if (info.address() == bluetoothAddress)
            m_deviceName = info.name();
    }
    if (m_deviceName.isEmpty())
        m_deviceName = m_deviceAddress;

    m_socket = new QBluetoothSocket(QBluetoothServiceInfo::RfcommProtocol, this);
    connect(m_socket, &QBluetoothSocket::connected,
            this, &BluetoothManager::onSocketConnected);
    connect(m_socket, &QBluetoothSocket::disconnected,
            this, &BluetoothManager::onSocketDisconnected);
    connect(m_socket, &QBluetoothSocket::errorOccurred,
            this, &BluetoothManager::onSocketError);
    connect(m_socket, &QBluetoothSocket::readyRead,
            this, &BluetoothManager::onReadyRead);

    setErrorString({});
    setState(Connecting);
    m_connectTimeout.start();
    m_socket->connectToService(bluetoothAddress, ServiceUuid, QIODevice::ReadWrite);
}

void BluetoothManager::disconnectFromDevice() {
    closeSocket();
    setState(Disconnected);
}

void BluetoothManager::sendMessage(const QByteArray &message) {
    if (m_state != Connected || !m_socket)
        return;
    m_socket->write(message + '\n');
}

void BluetoothManager::onSocketConnected() {
    m_connectTimeout.stop();
    m_receiveBuffer.clear();
    setState(Connected);
}

void BluetoothManager::onSocketDisconnected() {
    // Also reached when the phone closes the connection or goes out of range
    closeSocket();
    setState(Disconnected);
}

void BluetoothManager::onSocketError(QBluetoothSocket::SocketError error) {
    if (!m_socket)
        return;
    qWarning() << "Bluetooth socket error" << error << m_socket->errorString();

    switch (error) {
    case QBluetoothSocket::SocketError::ServiceNotFoundError:
        setErrorString(tr("The phone app is not running or not waiting for a connection"));
        break;
    case QBluetoothSocket::SocketError::HostNotFoundError:
        setErrorString(tr("Device not found"));
        break;
    default:
        setErrorString(m_socket->errorString());
        break;
    }

    closeSocket();
    setState(Disconnected);
}

// Splits the stream into lines; a message may arrive in several reads, and
// one read may hold several messages
void BluetoothManager::onReadyRead() {
    if (!m_socket)
        return;
    m_receiveBuffer.append(m_socket->readAll());

    qsizetype end;
    while ((end = m_receiveBuffer.indexOf('\n')) >= 0) {
        const QByteArray message = m_receiveBuffer.left(end).trimmed();
        m_receiveBuffer.remove(0, end + 1);
        if (!message.isEmpty())
            emit messageReceived(message);
        // A handler may have disconnected
        if (!m_socket)
            return;
    }

    if (m_receiveBuffer.size() > MaxMessageSize) {
        qWarning() << "Bluetooth: dropping" << m_receiveBuffer.size()
                   << "bytes without a line terminator";
        m_receiveBuffer.clear();
    }
}

void BluetoothManager::closeSocket() {
    m_connectTimeout.stop();
    m_receiveBuffer.clear();
    if (!m_socket)
        return;
    QBluetoothSocket *socket = m_socket;
    m_socket = nullptr;
    socket->disconnect(this);
    socket->abort();
    socket->deleteLater();
}

void BluetoothManager::setState(State state) {
    if (m_state == state)
        return;
    m_state = state;
    if (m_state == Disconnected) {
        m_deviceName.clear();
        m_deviceAddress.clear();
    }
    emit stateChanged();
}

void BluetoothManager::setDiscovering(bool discovering) {
    if (m_discovering == discovering)
        return;
    m_discovering = discovering;
    emit discoveringChanged();
}

void BluetoothManager::setErrorString(const QString &error) {
    if (m_errorString == error)
        return;
    m_errorString = error;
    emit errorStringChanged();
}
