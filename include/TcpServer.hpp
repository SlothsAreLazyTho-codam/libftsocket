#pragma once

#include <Socket.hpp>
#include <TcpClient.hpp>
#include <macros.h>

#include <algorithm>
#include <iostream>
#include <string>
#include <cstring>
#include <stdbool.h>
#include <poll.h>

#include <map>
#include <list>
#include <sstream>

#ifdef POLL_TIMEOUT
	#undef POLL_TIMEOUT
#endif

#define POLL_TIMEOUT 100

typedef	void (*messagefunc_t)(TcpClient *, const std::string &);

class TcpServer : public Socket
{
	private:
		std::vector<struct pollfd>	_pollfds;
		std::map<int, TcpClient*> _clients;

		inline void	handleClientEvent(TcpClient *client);

		int	handleClientConnection();
		int	removeClientFromList(int fd);

	public:
		TcpServer();
		virtual ~TcpServer();

		int		openup(const char *host, const char *port);
		void	loop(void);

		void	on_message_hook(messagefunc_t func);

	private:
		messagefunc_t	onMessage;
};