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

#include <exception>
#include <vector>

#define SERVER 1
#define CLIENT 2

class Socket
{
  private:
	struct addrinfo _hints;

  protected:
	struct pollfd fd;
	struct addrinfo *addrinfo;
	std::string _ip;
	std::string _port;

	int _open;

	virtual int open(const char *addr, const char *port, int type);

	void setConnected(int open);

  public:
	Socket();
	virtual ~Socket();

	void close(void);

	const std::string &getHost() const;
	const std::string &getPort() const;

	int isConnected(void);
	int isClosed(void);
	int getSocket(void);

	struct addrinfo *getAddress(void);
	struct pollfd getFileDescriptor();
};
