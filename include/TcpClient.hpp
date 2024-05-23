#pragma once

#include <IBaseSocket.hpp>
#include <macros.h>

#include <iostream>
#include <string>
#include <cstring>
#include <stdbool.h>

class TcpClient : public IBaseSocket
{
	private:
		std::string _buffer;

	public:
		TcpClient();
		TcpClient(int fd, sockaddr_in addr);
		virtual ~TcpClient();
		TcpClient &operator=(const TcpClient &other);

		int					connect(const char *host, const char *port);
		void				send(std::string buff);
		std::vector<char>	read();
		std::string			readString();

		const std::string	getBuffer();
		const int32_t		getBufferLength();
		void				resetBuffer();

};