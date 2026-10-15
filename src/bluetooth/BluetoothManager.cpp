#include "bluetooth/BluetoothManager.h"

void BluetoothManager::startBluetooth()
{
    if (m_bluetoothStatus)
        return;

    m_bluetoothStatus = true;
    emit bluetoothStatusChanged(m_bluetoothStatus);
}

void BluetoothManager::stopBluetooth()
{
    if (!m_bluetoothStatus)
        return;

    m_bluetoothStatus = false;
    emit bluetoothStatusChanged(m_bluetoothStatus);
}

bool BluetoothManager::bluetoothStatus() const
{
    return m_bluetoothStatus;
}