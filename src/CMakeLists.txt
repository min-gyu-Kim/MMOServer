

add_executable(MMO_Server 
    main.cpp
    types.hpp
    IOLoop/IOLoop_Epoll.hpp
    IOLoop/IOLoop_Epoll.cpp
    IOLoop/IOLoop.hpp
    IOLoop/IOLoop.cpp
    IOLoop/IConnection.hpp
    IOLoop/EventHandler_Linux.hpp
    IOLoop/EventHandler_Linux.cpp
)