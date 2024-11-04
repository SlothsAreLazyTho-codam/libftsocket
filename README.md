# LibSocket
A library made for my team project 'ft_irc'. The library aims to good functionality and simple tasks in C++20

# Installation
Make the project by using 'make', Include the library in your project as follow
```sh
c++ --std=c++20 Server.cpp Client.cpp main.cpp -Llibftsocket -lsocket
```

# Example
Server.hpp (refer back to tcp/TcpServer.hpp for the virtual functions)
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

        void Server::onConnect(const Client *client)
        {
            std::cout << client->getHost() << " connected to the server succesfully" << std::endl;
        }

        void Server::onDataReceived(const Client *client, const std::string &message)
        {
            std::cout << client->getHost() << ": " << message << std::endl;
        }

        void Server::onDisconnect(const Client *client)
        {
            std::cout << client->getHost() << " left the server" << std::endl;
        }
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

# Report
Please issue an problem or make a PR! Im happy to receive feedback

# Contact
Please refer back to my GitHub profile page for active connections!