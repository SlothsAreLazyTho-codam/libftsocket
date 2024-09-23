#pragma once

#include "tcp/TcpClient.hpp"

#include <Socket.hpp>
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

#define POLL_TIMEOUT 0

class TcpServer : public Socket
{
	private:
		std::vector<struct pollfd>	_pollfds;
		std::map<int, TcpClient*> _clients;

		inline void	handleClientEvent(TcpClient *client);

	public:
		TcpServer();
		virtual ~TcpServer();

		int		openup(const char *host, const char *port);
		void	loop(void);

		int	handleClientConnection();
		int	removeClientFromList(int fd);


	protected:
		virtual void onHandshake(TcpClient *);
		virtual void onMessage(TcpClient *, const std::string &);
		virtual void onDisconnect(int);
	
};