#pragma once

#include <IBaseSocket.hpp>
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

#define POLL_TIMEOUT -1

class TcpServer : public IBaseSocket
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
};