#pragma once

#include <cstring>
#include <iostream>
#include <string>

#include <fcntl.h>
#include <poll.h>
#include <stdbool.h>
#include <unistd.h>

#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>

class Socket
{
private:
	struct addrinfo _hints;

protected:
	const std::string _host;
	const std::string _port;

protected:
	std::string _stack;
	struct addrinfo *_addrinfo;
	pollfd _pollfd;

protected:
	bool open();

public:
	Socket(std::string stack, int fd, sockaddr_in info);
	Socket(std::string stack, pollfd fd, sockaddr_in info);
	Socket(std::string stack, const std::string &host, const std::string &port);
	virtual ~Socket();

public:
	const std::string &getStack() const;
	const std::string &getHost() const;
	const std::string &getPort() const;
	const int getFileDescriptor() const;
	pollfd &getPollFiledescriptor();
};