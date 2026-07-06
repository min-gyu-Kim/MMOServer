#pragma once

#include "../types.hpp"
#include "IConnection.hpp"
#include "IOLoop_Epoll.hpp"

namespace server {

class BaseEventHandler
{
public:
    BaseEventHandler(int fd, IOLoopEpollImpl* owner) : m_fd(fd), m_owner(owner) {}
    virtual ~BaseEventHandler() = default;

    int GetFD() const { return m_fd; }
    virtual bool HandleEvent(uint32 events) = 0;

private:
    BaseEventHandler(const BaseEventHandler&) = delete;
    BaseEventHandler& operator=(const BaseEventHandler&) = delete;

protected:
    int m_fd;
    IOLoopEpollImpl* m_owner;
};

class ListenHandler : public BaseEventHandler
{
public:
    ListenHandler(int fd, IOLoopEpollImpl* owner) : BaseEventHandler(fd, owner) {}
    virtual ~ListenHandler() = default;

    void SetListenCallback(ListenCallback callback) { m_callback = callback; }
    bool HandleEvent(uint32 events) override;

private:
    ListenCallback m_callback;
};

class ConnectHandler : public BaseEventHandler
{
public:
    ConnectHandler(int fd, IOLoopEpollImpl* owner) : BaseEventHandler(fd, owner) {}
    virtual ~ConnectHandler() = default;
    bool HandleEvent(uint32 events) override;
    void SetConnection(IConnection* connection) { m_connection = connection; }

private:
    IConnection* m_connection = nullptr;
    int32 m_writeOffset = 0;
    int32 m_readOffset = 0;
    std::byte m_readBuffer[65535];
};

class SocketIOHandler : public BaseEventHandler
{
public:
    SocketIOHandler(int fd, IOLoopEpollImpl* owner) : BaseEventHandler(fd, owner) {}
    virtual ~SocketIOHandler() = default;
    bool HandleEvent(uint32 events) override;
};

} // namespace server