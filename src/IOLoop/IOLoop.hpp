#pragma once

#include "../types.hpp"
#include "IConnection.hpp"

namespace server {

class IOLoop
{
public:
    IOLoop();
    ~IOLoop();

    void Run();
    void Stop();

    // TODO: return listener id
    bool Listen(const char* ip, uint16 port, ListenCallback callback);

private:
    void* m_impl;
};
} // namespace server