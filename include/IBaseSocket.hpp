#pragma once

#include <string>
#include <cstring>
#include <iostream>

#include <stdbool.h>
#include <unistd.h>
#include <poll.h>
#include <fcntl.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#include <exception>
#include <vector>

/***
 * Maybe put it in a struct? Like here?
*/
#ifdef DEBUG
	typedef struct
	{
		struct pollfd	fd;
		struct addrinfo	_hints;
		struct addrinfo	*addrinfo;
	}	socket_t;
#endif

#define SERVER 1
#define CLIENT 2

class IBaseSocket
{
	private:
		struct addrinfo	_hints;

	protected:
		struct pollfd	fd;
		struct addrinfo	*addrinfo;
		std::string		_ip;
		std::string		_port;
		
		int	_open;

		virtual int open(const char *addr, const char *port, int type);
		
		void setConnected(int open);
	public:
		IBaseSocket();
		virtual ~IBaseSocket();

		void close(void);

		int isConnected(void) const;
		int isClosed(void) const;
		int getSocket(void) const;

		const std::string& getHost() const;
		const std::string& getPort() const;

		struct addrinfo *getAddress(void) const;
		struct pollfd	getFileDescriptor() const;
};
