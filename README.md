# LibSocket
A library made for my team project 'ft_irc'. The library aims to good functionality and simple synchronized tasks in C++20

# Installation
Make the project by using 'make', Include the library in the compiler as follow
```sh
c++ --std=c++20 Server.cpp Client.cpp main.cpp -Llibftsocket -lsocket
```

# Example
Server.hpp
```cpp
#pragma once
#include "tcp/TcpServer.hpp"
#include "Client.hpp"

#include <string>

class Server : protected TcpServer<Client>
{
    public:
        Server(std::string _port): TcpServer<Client>("0.0.0.0", _port)
        { }

        ~Server()
        {}
};
```

Client.hpp
```cpp
#pragma once
#include <iostream>
#include "tcp/TcpClient.hpp"

class Client : protected TcpClient
{
    public:
        Client(int fd, sockaddr_in addr): TcpClient(fd, addr)
        { }

        ~Client()
        { }
};
```
