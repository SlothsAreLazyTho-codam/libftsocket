#pragma once

#include <iostream>
#include <string>
#include <memory>

#include <vector>
#include <map>

#include "Socket.hpp"
#include "tcp/TcpClient.hpp"

template<typename TClient>
class TcpServer : public Socket
{
	private:
		bool _running;

	private: /* Vectors */
		std::vector<struct pollfd>				_pollfds;
		std::map<int, std::shared_ptr<TClient>> _clients;

	public:
		TcpServer(const std::string ip, const std::string port): Socket("server_stack", ip, port),
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

		bool start()
		{
			int opt = 1;

			if (this->_running)
				return (true);
			
			if (!Socket::open())
				throw std::invalid_argument("bind() failed");

			if (setsockopt(this->_pollfd.fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
				return (false);

			if (bind(this->_pollfd.fd, this->_addrinfo->ai_addr, this->_addrinfo->ai_addrlen) < 0)
				return (false);
			
			if (listen(this->_pollfd.fd, 16) < 0)
				return (false);

			this->_pollfd.events = POLLIN | POLLOUT | POLLHUP;
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

	private:
		const std::shared_ptr<TClient> addClient(int fd, sockaddr_in addr)
		{
			auto client = std::make_shared<TClient>(fd, addr);
			this->_clients[fd] = client;
			return client;
		}

		/* Remove the client from polling */
		void removeClient(int fd)
		{
			std::cout << "File descriptor " << fd << "is leaving the poll" << std::endl;
			//std::erase_if(this->_pollfds, [fd](const pollfd &poll) { return poll.fd == fd; });
			this->_clients.erase(fd);
		}

		const std::shared_ptr<TClient> getClient(int fd)
		{
			if (fd <= STDERR_FILENO)
				return (nullptr);
			if (this->_clients.empty())
				return (nullptr); /* Return nullptr */
			return (this->_clients.at(fd));
		}

	public:
		void run()
		{
			while (_running) {
				if (poll(this->_pollfds.data(), this->_pollfds.size(), 5000) < 0) {
					return this->close();
				}

				for (size_t i = 0; i < this->_pollfds.size(); i++) {
					pollfd target = this->_pollfds[i];
					handleClientEvent(target.fd, target.revents);
				}
			}
		}

		const bool isRunning() const { return (this->_running); }
	
	private: /* Class implementations, Gonna be alot of funtions*/
		inline void	handleClientEvent(int fd, short event)
		{
			if (event & POLLIN)
			{
				if (fd == this->getFileDescriptor())
					handleClientConnection();
				else
				{
					std::shared_ptr<TcpClient> c = getClient(fd);

					std::cout << c->getFileDescriptor() << " is requestng data" << std::endl;

					if (c == nullptr)
						return;
					
					handleClientMessage(c);
				}
			}

			if (event & POLLOUT)
			{
				std::shared_ptr<TcpClient> c = this->getClient(fd);

				if (c->getBufferLength() != 0)
				{
					send(c->getFileDescriptor(),
						c->getBuffer().c_str(),
						c->getBufferLength(),
						0);

					c->flush();
				}
			}

			if (event & POLLHUP || event & POLLERR || event & POLLNVAL) {
				removeClient(fd);
			}			
		}

		inline void handleClientMessage(std::shared_ptr<TcpClient> client)
		{
			std::string delimiter = "\r\n";

			std::cout << "[Data] reading from filedescriptor" << client->getFileDescriptor() << std::endl;

			if (client->isClosed())
				return;

			try {
				std::vector<char> bytes = client->read();
				std::string buffer(bytes.data());

				if (!client->isConnected()) {
					removeClient(client->getFileDescriptor());
					return;
				}
				
				size_t pos = 0;
				while ((pos = buffer.find(delimiter)) != std::string::npos) {
					//this->onMessage(client, buffer.substr(0, pos));
					std::cout << "[Incoming] " << buffer.substr(0, pos) << std::endl;
					buffer.erase(0, pos + delimiter.length());
				}
			}
			catch (std::exception &ex) {
				removeClient(client->getFileDescriptor());
			}
		}

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

			std::shared_ptr<TcpClient> client = addClient(clientFd, client_addrin);
			this->_pollfds.push_back(client->getPollFiledescriptor());
			return (true);
		}
};