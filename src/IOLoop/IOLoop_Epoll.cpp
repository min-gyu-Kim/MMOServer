#include "IOLoop_Epoll.hpp"
#include "../types.hpp"
#include "EventHandler_Linux.hpp"
#include <thread>

#include <unistd.h>
#include <sys/epoll.h>
#include <sys/eventfd.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <string.h>
#include <stdio.h>
#include <signal.h>

namespace server {

namespace {
thread_local IOLoopEpollImpl::_ThreadContext* threadContext = nullptr;
}

IOLoopEpollImpl::IOLoopEpollImpl() {}
IOLoopEpollImpl::~IOLoopEpollImpl() {}

void IOLoopEpollImpl::Run()
{
    std::thread::id thisThreadID = std::this_thread::get_id();

    {
        std::lock_guard<std::mutex> lock(m_lock);
        auto it = m_threadContexts.insert({thisThreadID, _ThreadContext{}});
        if (!it.second) {
            return;
        }

        threadContext = &it.first->second;
    }

    int epollFd = epoll_create1(0);
    if (epollFd == -1) {
        return;
    }

    int eventFd = eventfd(0, EFD_NONBLOCK);
    if (eventFd == -1) {
        close(epollFd);
        return;
    }

    epoll_event event{};
    event.events = EPOLLIN;
    event.data.fd = eventFd;
    if (epoll_ctl(epollFd, EPOLL_CTL_ADD, eventFd, &event) == -1) {
        close(epollFd);
        close(eventFd);
    }

    _ThreadContext context;
    m_threadContexts[thisThreadID] = context;
    threadContext = &m_threadContexts[thisThreadID];

    threadContext->epoll_fd = epollFd;
    threadContext->event_fd = eventFd;

    // TODO 개선하기
    {
        std::lock_guard<std::mutex> lock(m_lock);
        for (ListenHandler* listenHandler : m_listenHandlers) {
            epoll_event event{};
            event.events = EPOLLIN;
            event.data.ptr = listenHandler;

            if (epoll_ctl(threadContext->epoll_fd, EPOLL_CTL_ADD, listenHandler->GetFD(), &event) ==
                -1) {
                close(listenHandler->GetFD());
                delete listenHandler;
                // TODO: listener handler ref count. multi epoll threads may share the same listen
                // handler. We should only delete it when all threads have removed it.
            }
        }
    }

    PollEvents();

    {
        std::lock_guard<std::mutex> lock(m_lock);
        m_threadContexts.erase(thisThreadID);

        close(threadContext->epoll_fd);
        close(threadContext->event_fd);

        threadContext = nullptr;
    }
}

void IOLoopEpollImpl::Stop()
{
    for (auto& pair : m_threadContexts) {
        const int addedValue = 1;
        write(pair.second.event_fd, &addedValue, sizeof(uint64));
    }

    m_threadContexts.clear();
}

void IOLoopEpollImpl::PollEvents()
{
    constexpr int32 MAX_EVENTS = 128;
    bool running = true;
    while (running) {
        struct epoll_event events[MAX_EVENTS];
        int nfds = epoll_wait(threadContext->epoll_fd, events, MAX_EVENTS, -1);
        if (nfds == -1) {
            if (errno == EINTR) {
                continue;
            }
            break;
        }

        for (int i = 0; i < nfds; ++i) {
            if (events[i].data.fd == threadContext->event_fd) {
                uint64 value;
                read(threadContext->event_fd, &value, sizeof(uint64));
                running = false;
            } else {
                BaseEventHandler* handler = reinterpret_cast<BaseEventHandler*>(events[i].data.ptr);
                if (handler) {
                    handler->HandleEvent(events[i].events);
                }
            }
        }
    }
}

bool IOLoopEpollImpl::Listen(const char* ip, uint16 port, ListenCallback callback)
{
    int listenFd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenFd == -1) {
        return false;
    }

    const int32 reusePort = 1;
    int32 result = setsockopt(listenFd, SOL_SOCKET, SO_REUSEPORT, &reusePort, sizeof(reusePort));
    if (result == -1) {
        close(listenFd);
        return false;
    }

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    inet_pton(AF_INET, ip, &addr.sin_addr);

    if (bind(listenFd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) == -1) {
        close(listenFd);
        return false;
    }

    if (listen(listenFd, SOMAXCONN) == -1) {
        close(listenFd);
        return false;
    }

    int32 flags = fcntl(listenFd, F_GETFL, 0);
    fcntl(listenFd, F_SETFL, flags | O_NONBLOCK);

    ListenHandler* listenHandler = new ListenHandler(listenFd, this);
    listenHandler->SetListenCallback(callback);

    epoll_event event{};
    event.events = EPOLLIN;
    event.data.ptr = listenHandler;

    {
        std::lock_guard<std::mutex> lock(m_lock);

        for (auto& pair : m_threadContexts) {

            if (epoll_ctl(pair.second.epoll_fd, EPOLL_CTL_ADD, listenFd, &event) == -1) {
                close(listenFd);
                delete listenHandler;
                return false;
            }
        }

        m_listenHandlers.push_back(listenHandler);
    }

    return true;
}

bool IOLoopEpollImpl::AddHandler(ConnectHandler* handler)
{
    epoll_event event{};
    event.events = EPOLLIN | EPOLLRDHUP | EPOLLERR;
    event.data.ptr = handler;

    if (epoll_ctl(threadContext->epoll_fd, EPOLL_CTL_ADD, handler->GetFD(), &event) == -1) {
        close(handler->GetFD());
        delete handler;
        return false;
    }

    return true;
}

} // namespace server