#pragma once
#include <vector>

#include "Socket.hpp"

class TcpClient : public Socket
{
	private:
		bool		_connected;
		std::string	_buffer;

	public:
		TcpClient(int fd, sockaddr_in addr);
		TcpClient(pollfd fd, sockaddr_in addr);
		virtual ~TcpClient();

		bool connect(std::string address, std::string port);

		void				send(std::string arg);
		std::vector<char>	read();
		void				close();

	public:
		const bool isConnected() const;
		const bool isClosed() const;

	public:
		void flush();
		[[nodiscard]] const std::string &getBuffer() const;
		[[nodiscard]] const int getBufferLength() const;
};