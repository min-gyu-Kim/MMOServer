#include "EventHandler_Linux.hpp"

#include <sys/epoll.h>
#include <arpa/inet.h>
#include <unistd.h>

namespace server {

bool ListenHandler::HandleEvent(uint32 events)
{
    if (events & EPOLLIN) {
        int listenFd = GetFD();
        sockaddr_in clientAddr{};
        socklen_t addrLen = sizeof(clientAddr);
        int clientFd = accept(listenFd, reinterpret_cast<sockaddr*>(&clientAddr), &addrLen);
        if (clientFd != -1) {
            if (m_callback) {
                IConnection* connection =
                    m_callback(inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));

                if (connection) {
                    ConnectHandler* connectHandler = new ConnectHandler(clientFd, m_owner);
                    connectHandler->SetConnection(connection);

                    return m_owner->AddHandler(connectHandler);

                } else {
                    close(clientFd);
                }
            }
        }
    }
    return true;
}

bool ConnectHandler::HandleEvent(uint32 events)
{
    if (events & EPOLLIN) {
        int bytesRead =
            read(GetFD(), m_readBuffer + m_writeOffset, sizeof(m_readBuffer) - m_writeOffset);
        if (bytesRead > 0) {
            m_writeOffset += bytesRead;
            const int32 canReadBytes = m_writeOffset - m_readOffset;

            if (m_connection) {
                BufferView bufferView(m_readBuffer + m_readOffset, canReadBytes);
                int32 readBytes = m_connection->OnRead(&bufferView);
                m_readOffset += readBytes;
                if (m_readOffset == m_writeOffset) {
                    m_readOffset = 0;
                    m_writeOffset = 0;
                }
            }
        } else if (bytesRead == 0) {
            // Connection closed by peer
            close(GetFD());
            delete this;
            return false;
        } else {
            // Error occurred
            if (m_connection) {
                m_connection->OnDisconnect(errno);
            }
            close(GetFD());
            delete this;
            return false;
        }
    }

    if (events & EPOLLOUT) {
        // Handle write events if needed
    }

    if (events & (EPOLLERR | EPOLLRDHUP)) {
        // Handle error or hang-up events
        if (m_connection) {
            m_connection->OnDisconnect(errno);
        }
        close(GetFD());
        delete this;
        return false;
    }

    return true;
}

} // namespace server