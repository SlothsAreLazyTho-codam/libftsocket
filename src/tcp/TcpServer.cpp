#include "TcpServer.hpp"
#include "Utils.hpp"

#include <fstream>

TcpServer::TcpServer() : _pollfds(0), _clients()
{ }

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
#ifdef DEBUG
	std::cout << "\t" << BOLD_MAGENTA << "↳ " << RESET << "Debug Mode: enabled" << std::endl;
#endif
	std::cout << std::endl;
}

std::string generate_http_response(const std::string body) {
    std::ostringstream response_stream;
    response_stream << "HTTP/1.1 200 OK\r\n";
    response_stream << "Content-Type: text/html\r\n";
    response_stream << "Content-Length: " << body.length() << "\r\n";
    response_stream << "\r\n";
    response_stream << body;
    return response_stream.str();
}

std::string build_response() {
    std::fstream file("./index.html");
    
	if (file.fail()) {
        return ("<html>No page found!</html>");
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    return generate_http_response(buffer.str());
}

int TcpServer::openup(const char *host, const char *port)
{
	int opt = 1;

	if (this->isConnected())
		return (1);

	NOT_BELOW_ZERO(Socket::open(host, port, SERVER));

	NOT_BELOW_ZERO(setsockopt(this->fd.fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)));

	NOT_BELOW_ZERO(bind(
		this->getSocket(),
		this->getAddress()->ai_addr,
		this->getAddress()->ai_addrlen));

	NOT_BELOW_ZERO(listen(this->fd.fd, 420));

	this->_pollfds.push_back(this->fd);
	this->setConnected(true);
	print_socket_information(*this);
	return (1);
}

void TcpServer::loop(void)
{
	std::cout << BOLD_MAGENTA << "Ready to accept connections." << RESET << std::endl;
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
			this->m_onMessage(client, buffer.substr(0, pos));
			buffer.erase(0, pos + delimiter.length());
		}
	}
	catch (std::invalid_argument &ex)
	{
		LOG_DEBUG("Throwing exception, cause: " << ex.what());
		this->removeClientFromList(client->getSocket());
	}
}

// Clean this mess up!
int TcpServer::handleClientConnection()
{
	sockaddr_in client_addrin;
	socklen_t client_addrlen = sizeof(client_addrin);
	int32_t clientFd = accept(this->fd.fd, (sockaddr *)&client_addrin, &client_addrlen);

	NOT_BELOW_ZERO(clientFd);
	NOT_BELOW_ZERO(fcntl(clientFd, F_SETFL, O_NONBLOCK));

	TcpClient *client = new TcpClient(clientFd, client_addrin);

	this->_pollfds.push_back(client->getFileDescriptor());
	this->_clients.insert({client->getSocket(), client});
	{
		std::cout << BOLD_MAGENTA << "(" << RESET << this->_pollfds.size() - 1 << BOLD_MAGENTA << ") " << \
			client->getHost() << RESET << " joined the server" << std::endl;
	}
	this->m_onHanshake(client, "no_data_provided_yet!"); //TODO Wait for the first message from the user.
	return (1);
}

int TcpServer::removeClientFromList(int fd)
{
	TcpClient *client = this->_clients.at(fd);

	for (std::vector<pollfd>::iterator begin = _pollfds.begin(); begin != this->_pollfds.end(); begin++)
	{
		if (begin->fd == fd)
		{
			LOG_DEBUG("Removed socket : " << begin->fd << " from the pollers list");
			this->_pollfds.erase(begin);
			break;
		}
	}

	this->_clients.erase(fd);
	LOG_DEBUG(client->getHost() << " left the server");
	delete client;
	return (1);
}

void	TcpServer::on_message_hook(messagefunc_t func)
{
	this->m_onMessage = func;
}

void	TcpServer::on_handshake_hook(handshakefunc_t func)
{
	this->m_onHanshake = func;
}