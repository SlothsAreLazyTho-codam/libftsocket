#pragma once

#include <Socket.hpp>
#include <macros.h>

#include <iostream>
#include <string>
#include <cstring>
#include <stdbool.h>

class TcpClient : public Socket
{
	private:
		std::string _buffer;

	public:
		TcpClient();
		TcpClient(int fd, sockaddr_in addr);
		virtual ~TcpClient();
		TcpClient(const TcpClient &client);
		TcpClient &operator=(const TcpClient &other);

		int					connect(const char *host, const char *port);
		void				send(std::string buff);
		std::vector<char>	read();
		std::string			readString();

		std::string			getBuffer() const;
		int32_t				getBufferLength() const;
		void				resetBuffer();

};