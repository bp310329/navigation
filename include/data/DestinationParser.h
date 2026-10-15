#pragma once

#include <QByteArray>
#include <optional>

#include "Destination.h"

class DestinationParser {
public:
    static std::optional<Destination> parse(const QByteArray &data);
};