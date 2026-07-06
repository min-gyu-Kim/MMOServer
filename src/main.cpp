
#include "IOLoop/IOLoop.hpp"
#include "types.hpp"
#include <stdio.h>

class MyConnection : public server::IConnection
{
public:
    server::int32 OnRead(const server::BufferView* buffer) override
    {
        printf("Received %d bytes\n", buffer->GetSize());
        return buffer->GetSize(); // Return the number of bytes processed
    }

    void OnWrite(server::int32 len) override
    {
        // Handle write completion
    }

    void OnError(server::int32 errNo) override
    {
        // Handle error
    }
};

int main(int argc, char* argv[])
{
    server::IOLoop loop;

    loop.Listen("0.0.0.0", 9000, [](const char* ip, server::uint16 port) -> server::IConnection* {
        MyConnection* connection = new MyConnection();

        printf("New connection from %s:%d\n", ip, port);
        return connection; // Return a valid IConnection instance here
    });
    loop.Run();

    return 0;
}