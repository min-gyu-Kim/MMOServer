#pragma once

#include "../types.hpp"

using AcceptorID = server::uint64;
using ConnectionID = server::uint64;

constexpr AcceptorID INVALID_ACCEPTOR_ID = 0xFFFFFFFFFFFFFFFF;

using OnRecv = size_t(ConnectionID id, const char* buffer, size_t bufferLen);
using OnSend = void(ConnectionID id, size_t sendSize);
using OnConnect = bool(ConnectionID id);
using OnDisconnect = void(ConnectionID id);

struct ConnectionHandler
{
    OnRecv recvHandler;
    OnSend sendHandler;
    OnConnect connectHandler;
    OnDisconnect disconnectHandler;
};

class INetwork
{
public:
    virtual ~INetwork() = default;

    using OnAcceptFunction = bool(const char* IPv4, server::uint16 port, ConnectionID identity);
    virtual AcceptorID Listen(const char* ip, server::uint16 port,
                              ConnectionHandler connectionHandler,
                              OnAcceptFunction acceptHandler) = 0;

    virtual void CancelListen(AcceptorID id);

    virtual bool Connect(const char* destIp, server::uint16 port,
                         ConnectionHandler connectionHandler);

    virtual void Send(ConnectionID id, const char* buffer, size_t bufSize);
    virtual void Disconnect(ConnectionID id);

    virtual void Run();
    virtual void Stop();
    // virtual void SetSamplingStatistics();
};
