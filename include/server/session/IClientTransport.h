#pragma once
#include <string>

using ConnectionId = int;

class IClientTransport {
public:
    virtual ~IClientTransport() = default;
    virtual void sendTo(ConnectionId connection, const std::string& text) = 0;
};
