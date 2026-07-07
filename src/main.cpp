
#include "IOLoop/IOLoop.hpp"
#include "types.hpp"
#include <stdio.h>
#include <thread>

class MyConnection : public server::IConnection
{
public:
    server::int32 OnRead(const server::BufferView* buffer) override
    {
        printf("%llu Received %d bytes\n", std::this_thread::get_id(), buffer->GetSize());
        return buffer->GetSize(); // Return the number of bytes processed
    }

    void OnWrite(server::int32 len) override
    {
        // Handle write completion
    }

    void OnDisconnect(server::int32 errNo) override
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

    std::vector<std::thread> ioThreads;
    const int numThreads = std::thread::hardware_concurrency();
    for (int i = 0; i < numThreads; ++i) {
        ioThreads.emplace_back([&loop]() { loop.Run(); });
    }

    for (auto& thread : ioThreads) {
        if (thread.joinable()) {
            thread.join();
        }
    }

    loop.Stop();

    return 0;
}