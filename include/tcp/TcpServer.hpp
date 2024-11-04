#pragma once

#include <iostream>
#include <string>
#include <memory>

#include <vector>
#include <map>

#include "Socket.hpp"
#include "tcp/TcpClient.hpp"

template <typename TClient>
class TcpServer : public Socket
{
private:
	bool _running;

private: /* Vectors */
	std::vector<struct pollfd> _pollfds;
	std::map<int, TClient *> _clients;

public:
	TcpServer(const std::string ip, const std::string port) : Socket("server_stack", ip, port),
															  _running(false),
															  _pollfds(0), _clients()
	{
		static_assert(std::is_base_of<TcpClient, TClient>::value, "TClient must inherit from TcpClient");
	}

	~TcpServer()
	{
		this->_pollfds.clear();
		this->_clients.clear();
		this->close();
	}

public:
	virtual void onConnect(TClient *client) { (void)client; }
	virtual void onDataReceived(TClient *client, const std::string &message)
	{
		(void)client;
		(void)message;
	}
	virtual void onDisconnect(TClient *client) { (void)client; }

public:
	bool start()
	{
		int opt = 1;

		if (this->_running)
			return (true);

		if (!Socket::open())
			throw std::invalid_argument("bind() failed");

		if (setsockopt(this->_pollfd.fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
			throw std::invalid_argument("setsockopt() failed");

		if (bind(this->_pollfd.fd, this->_addrinfo->ai_addr, this->_addrinfo->ai_addrlen) < 0)
			throw std::invalid_argument("bind() failed");

		if (listen(this->_pollfd.fd, 16) < 0)
			throw std::invalid_argument("listen() failed");

		this->_pollfds.push_back(this->_pollfd);
		this->_running = true;
		return (_running);
	}

	void close()
	{
		if (!isRunning())
			return;
		this->_running = false;
	}

public:
	void run()
	{
		while (_running)
		{
			if (poll(this->_pollfds.data(), this->_pollfds.size(), 5000) < 0)
			{
				return this->close();
			}

			for (size_t i = 0; i < this->_pollfds.size(); i++)
			{
				pollfd target = this->_pollfds[i];
				handleClientEvent(target.fd, target.revents);
			}
		}
	}

	const bool isRunning() const { return (this->_running); }

protected:
	/* Add the client to the socket */
	TClient *addClient(int fd, sockaddr_in addr)
	{
		TClient *client = new TClient(fd, addr);
		this->_clients.emplace(fd, client);
		return client;
	}

	/* Remove the client from polling */
	void removeClient(int fd)
	{
		TClient *client = this->getClient(fd);
		this->onDisconnect(client);

		// Remove the client from this point.
		std::erase_if(this->_pollfds, [fd](const pollfd &poll)
					  { return poll.fd == fd; });
		this->_clients.erase(fd);
	}

	TClient *getClient(int fd)
	{
		if (fd <= STDERR_FILENO)
			return (nullptr);
		if (this->_clients.empty())
			return (nullptr); /* Return nullptr */
		return (this->_clients.at(fd));
	}

private: /* Class implementations, Gonna be alot of funtions*/
	/**
	 * @brief Handles events for a client socket.
	 *
	 * This function processes different types of events for a client socket
	 * identified by its file descriptor. It handles incoming data, outgoing data,
	 * and error conditions.
	 *
	 * @param fd The file descriptor of the client socket.
	 * @param event The event type, which can be a combination of POLLIN, POLLOUT,
	 *              POLLHUP, POLLERR, and POLLNVAL.
	 *
	 * - If the event is POLLIN, it handles incoming data. If the file descriptor
	 *   matches the server's file descriptor, it handles a new client connection.
	 *   Otherwise, it processes the incoming message from the client.
	 * - If the event is POLLOUT, it handles outgoing data. It sends any buffered
	 *   data to the client and flushes the buffer.
	 * - If the event is POLLHUP, POLLERR, or POLLNVAL, it removes the client.
	 */
	inline void handleClientEvent(int fd, short event)
	{
		if (event & POLLIN)
		{
			if (fd == this->getFileDescriptor())
				handleClientConnection();
			else
			{
				TcpClient *client = getClient(fd);

				if (client == nullptr)
					return;

				handleClientMessage(client);
			}
			return;
		}

		if (event & POLLOUT)
		{
			TcpClient *client = this->getClient(fd);

			if (client->isClosed())
				return;

			if (client->getBufferLength() != 0)
			{
				send(client->getFileDescriptor(),
					 client->getBuffer().data(),
					 client->getBufferLength(),
					 0);

				client->flush();
			}

			return;
		}

		if (event & POLLHUP || event & POLLERR || event & POLLNVAL)
		{
			removeClient(fd);
			return;
		}
	}

	/**
	 * @brief Handles incoming messages from a connected TCP client.
	 *
	 * This function reads data from the specified TCP client, processes the data
	 * to extract complete messages based on a delimiter, and handles the messages.
	 * If the client is closed or an exception occurs during reading, the client
	 * is removed.
	 *
	 * @param client A shared pointer to the TcpClient object representing the connected client.
	 */
	inline void handleClientMessage(TcpClient *client)
	{
		std::string delimiter = "\r\n";
		int pos = 0;

		try
		{
			std::vector<char> bytes = client->read();
			std::string buffer(bytes.data());

			if (client->isClosed())
			{
				removeClient(client->getFileDescriptor());
				return;
			}

			TClient *baseClient = this->getClient(client->getFileDescriptor());

			while ((pos = buffer.find(delimiter)) != std::string::npos)
			{
				this->onDataReceived(baseClient, buffer.substr(0, pos));
				buffer.erase(0, pos + delimiter.length());
			}
		}
		catch (std::exception &ex)
		{
			removeClient(client->getFileDescriptor());
		}
	}

	/**
	 * @brief Handles an incoming client connection.
	 *
	 * This function accepts a new client connection on the server's file descriptor,
	 * sets the client socket to non-blocking mode, and adds the client to the list
	 * of managed clients.
	 *
	 * @return true if the client connection was successfully handled and added;
	 *         false otherwise.
	 */
	inline bool handleClientConnection()
	{
		sockaddr_in client_addrin;
		socklen_t client_addrlen = sizeof(client_addrin);

		int32_t clientFd = accept(this->getFileDescriptor(),
								  (sockaddr *)&client_addrin, &client_addrlen);

		if (clientFd < 2)
			return (false);

		if (fcntl(clientFd, F_SETFL, O_NONBLOCK) < 0)
			return (false);

		TClient *client = addClient(clientFd, client_addrin);
		this->_pollfds.push_back(
			static_cast<TcpClient *>(client)->getPollFiledescriptor());
		this->onConnect(client);
		return (true);
	}
};