#pragma once

#include "../types.hpp"
#include <functional>

namespace server {

class IConnection
{
public:
    virtual ~IConnection() = default;

    virtual int32 OnRead(const class BufferView* buffer) = 0;
    virtual void OnWrite(int32 len) = 0;
    virtual void OnDisconnect(int errNo) = 0;
};

using ListenCallback = std::function<IConnection*(const char* ip, uint16 port)>;
} // namespace server