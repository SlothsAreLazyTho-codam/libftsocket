#include "TcpClient.hpp"
#include "Utils.hpp"

#define MAX_BUFFER 1024

TcpClient::TcpClient()
{
}

TcpClient::TcpClient(int fd, sockaddr_in addr)
{
	//Set the file descriptor up manually
	this->fd.fd = fd;
	this->fd.events = POLLIN | POLLOUT;
	this->fd.revents = 0;

	this->_ip = Utils::addrintoipv4(addr.sin_addr);
	this->_port = addr.sin_port;
	this->setConnected(true);
}

TcpClient::~TcpClient()
{
	this->_buffer.clear();
}

TcpClient &TcpClient::operator=(const TcpClient &other)
{
	this->fd.fd = other.fd.fd;
	this->fd.events = other.fd.events;
	this->fd.revents = 0;
	this->_ip = other._ip;
	this->_port = other._port;
	this->_open = other._open;
	LOG_DEBUG("Copy assignment operator used on TcpClient");
	return (*this);
}

int TcpClient::connect(const char *host, const char *port)
{
	if (this->isConnected())
		return (1);

	NOT_BELOW_ZERO(IBaseSocket::open(host, port, CLIENT));

	NOT_BELOW_ZERO(::connect(
		this->fd.fd,
		this->addrinfo->ai_addr,
		this->addrinfo->ai_addrlen));

	this->setConnected(true);
	return (1);
}

void TcpClient::send(std::string buff)
{
	if (this->isClosed())
		return;

	if (buff.find("\r\n") == std::string::npos)
		buff += "\r\n";

	this->_buffer += buff;
}

/// @brief Read bytes from socket
/// @return Byte array in a char pointer.
std::vector<char> TcpClient::read()
{
	std::vector<char>	response(0);
	char				buffer[MAX_BUFFER];
	int32_t				bytes_read = 1;

	if (this->isClosed())
	{
		throw std::invalid_argument("Cannot write to closed socket.");
	}

	while (bytes_read > 0)
	{
		bytes_read = recv(this->fd.fd, &buffer, 1024, 0);
		
		if (bytes_read == 0)
		{
			throw std::invalid_argument("connection reset by peer.");
		}

		if (bytes_read <= -1)
		{
			if (errno == EINTR)
				continue;
			else if (errno == EWOULDBLOCK || errno == EAGAIN)
				break;
			else
				throw std::invalid_argument("recv() failed");
		}
		LOG_DEBUG("Reading..." << bytes_read);
		response.insert(response.end(), buffer, buffer + bytes_read);
	}

	LOG_DEBUG("Done reading(" << bytes_read << ")");
	return (response);
}

std::string TcpClient::readString()
{
	try {
		std::vector<char> data = read();

		if (data.empty())
			throw std::invalid_argument("no data was received by peer");

		data.push_back(0);
		return (data.data());
	}
	catch (std::exception &ex)
	{
		return (std::string());
	}

	return (nullptr);
}


const std::string	TcpClient::getBuffer()
{
	return (this->_buffer);
}

const int32_t		TcpClient::getBufferLength()
{
	return (this->_buffer.length());
}

void	TcpClient::resetBuffer(void)
{
	this->_buffer.clear();
}