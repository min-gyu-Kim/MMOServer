#pragma once

#include "../types.hpp"
#include "IConnection.hpp"

#include <mutex>
#include <thread>
#include <unordered_map>
#include <vector>

namespace server {

class ListenHandler;
class ConnectHandler;

class IOLoopEpollImpl
{
public:
    IOLoopEpollImpl();
    ~IOLoopEpollImpl();

    void Run();
    void Stop();
    bool Listen(const char* ip, uint16 port, ListenCallback callback);

    struct _ThreadContext
    {
        int epoll_fd;
        int event_fd;
    };

    bool AddHandler(ConnectHandler* handler);

private:
    void PollEvents();

private:
    std::mutex m_lock;
    std::unordered_map<std::thread::id, _ThreadContext> m_threadContexts;
    std::vector<ListenHandler*> m_listenHandlers;
};
} // namespace server