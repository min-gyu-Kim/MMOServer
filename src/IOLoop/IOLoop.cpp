#include "IOLoop.hpp"

#ifdef __linux__
#include "IOLoop_Epoll.hpp"
using Impl = server::IOLoopEpollImpl;
#endif

namespace server {

IOLoop::IOLoop() : m_impl(new Impl()) {}
IOLoop::~IOLoop()
{
    delete static_cast<Impl*>(m_impl);
}

void IOLoop::Run()
{
    static_cast<Impl*>(m_impl)->Run();
}

void IOLoop::Stop()
{
    static_cast<Impl*>(m_impl)->Stop();
}

bool IOLoop::Listen(const char* ip, uint16 port, ListenCallback callback)
{
    return static_cast<Impl*>(m_impl)->Listen(ip, port, callback);
}

} // namespace server