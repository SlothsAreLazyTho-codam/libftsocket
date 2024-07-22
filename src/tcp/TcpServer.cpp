#include "TcpServer.hpp"
#include "Utils.hpp"

#include <fstream>

/**
 * @brief Default constructor for the TcpServer class.
 */
TcpServer::TcpServer() : _pollfds(0), _clients()
{ }

/**
 * @brief Destructor for the TcpServer class.
 * 
 * This destructor clears the pollfds and clients vectors.
 */
TcpServer::~TcpServer()
{
	this->_pollfds.clear();
	this->_clients.clear();
}

inline void print_socket_information(TcpServer &server)
{
	std::cout << std::endl;
	std::cout << "\t" << BOLD_MAGENTA << "▲ " << "FTSocket " << FTSOCK_VERSION << RESET << std::endl;
	std::cout << "\t" << BOLD_MAGENTA << "↳ " << RESET << "Local: " << server.getHost() << ":" << server.getPort() << std::endl;
	std::cout << "\t" << BOLD_MAGENTA << "↳ " << RESET << "Debug Mode: enabled" << std::endl;
	std::cout << std::endl;
}

/**
 * @brief Opens up a TCP server connection.
 *
 * This function opens a TCP server connection on the specified host and port.
 * It performs the following steps:
 * 1. Checks if the server is already connected. If yes, returns 1.
 * 2. Opens a socket using the Socket::open function.
 * 3. Sets the SO_REUSEADDR option on the socket.
 * 4. Binds the socket to the specified address.
 * 5. Starts listening for incoming connections.
 * 6. Adds the socket to the list of pollfds.
 * 7. Sets the connected flag to true.
 *
 * @param host The host to bind the server to.
 * @param port The port to bind the server to.
 * @return Returns 1 on success, -1 on failure.
 */
int TcpServer::openup(const char *host, const char *port)
{
	int opt = 1;

	if (this->isConnected())
		return (1);

	if (Socket::open(host, port, SERVER) < 0)
		return (-1);

	if (setsockopt(this->fd.fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
		return (-1);

	if (bind(this->getSocket(), this->getAddress()->ai_addr, this->getAddress()->ai_addrlen) < 0)
		return (-1); 

	if (listen(this->fd.fd, 5) < 0)
		return (-1);

	this->_pollfds.push_back(this->fd);
	this->setConnected(true);

#ifdef DEBUG
	print_socket_information(*this);
#else
	std::cout << "(!) Ready to accept connections";
#endif

	return (1);
}

/**
 * @brief Enters the main loop of the TCP server, continuously accepting connections and handling events.
 *
 * This function starts the main loop of the TCP server. It continuously checks for events on the registered file descriptors
 * using the `poll` function. It handles various events such as client disconnections, errors, incoming data, and outgoing data.
 * The loop continues until the server is no longer running.
 *
 * @throws std::runtime_error if the `poll` function fails.
 */
void TcpServer::loop(void)
{
	int	x;

	while (this->isConnected())
	{
		if (poll(this->_pollfds.data(), this->_pollfds.size(), POLL_TIMEOUT) < 0)
		{
			throw std::runtime_error("poll() failed");
		}
		for (size_t i = 0; i < this->_pollfds.size(); i++)
		{
			pollfd target = this->_pollfds[i];
			
			if (target.revents & POLLHUP
				|| target.revents & POLLERR
				|| target.revents & POLLNVAL)
			{
				this->removeClientFromList(target.fd);
			}
			else if (target.revents & POLLIN)
			{
				if (target.fd == this->fd.fd)
					handleClientConnection();
				else
					handleClientEvent(this->_clients.at(target.fd));
			}
			else if (target.revents & POLLOUT)
			{
				TcpClient *client = this->_clients.at(target.fd);
				
				x = send(target.fd, 
					client->getBuffer().c_str(),
					client->getBufferLength(), 0);

				if (x < 0)
				{
					this->removeClientFromList(target.fd);
					break;
				}

				client->resetBuffer();
			}
		}
	}
}

inline void TcpServer::handleClientEvent(TcpClient *client)
{
	std::string delimiter = "\n";
	try
	{
		std::string buffer = client->readString();

		if (client->isClosed())
		{
			removeClientFromList(client->getSocket());
			return;
		}

		size_t pos = 0;
		while ((pos = buffer.find(delimiter)) != std::string::npos)
		{
			this->onMessage(client, buffer.substr(0, pos));
			buffer.erase(0, pos + delimiter.length());
		}
	}
	catch (std::invalid_argument &ex)
	{
		LOG_DEBUG("Throwing exception, cause: " << ex.what());
		this->removeClientFromList(client->getSocket());
	}
}

inline int TcpServer::handleClientConnection()
{
	sockaddr_in client_addrin;
	socklen_t client_addrlen = sizeof(client_addrin);
	int32_t clientFd = accept(this->fd.fd, (sockaddr *)&client_addrin, &client_addrlen);

	if (clientFd < 2)
		return (-1);

	if (fcntl(clientFd, F_SETFL, O_NONBLOCK) < 0)
		return (-1);

	TcpClient *client = new TcpClient(clientFd, client_addrin);

	this->_pollfds.push_back(client->getFileDescriptor());
	this->_clients.insert({client->getSocket(), client});
	{
		std::cout << BOLD_MAGENTA << "(" << RESET << this->_pollfds.size() - 1 << BOLD_MAGENTA << ") " << \
			client->getHost() << RESET << " joined the server" << std::endl;
	}
	this->onHandshake(client);
	return (1);
}

int TcpServer::removeClientFromList(int fd)
{
	TcpClient *client = this->_clients.at(fd);

	for (std::vector<pollfd>::iterator begin = _pollfds.begin(); begin != this->_pollfds.end(); begin++)
	{
		if (begin->fd != fd)
			continue;
		this->_pollfds.erase(begin);	
		break;
	}

	this->_clients.erase(fd);
	LOG_DEBUG(client->getHost() << " left the server");
	delete client;
	this->onDisconnect(fd);
	return (1);
}

//void	TcpServer::setHandshakeHook(handshakefunc_t func) { this->m_onHanshake = func; }
//void	TcpServer::setMessageHook(messagefunc_t func) { this->m_onMessage = func; }
//void	TcpServer::setDisconnectHook(disconnectfunc_t func) { this->m_onDisconnect = func; }

void TcpServer::onHandshake(TcpClient *a) {(void) a;}
void TcpServer::onMessage(TcpClient *a, const std::string &b) { (void) a; (void) b;}
void TcpServer::onDisconnect(int a) { (void) a;}