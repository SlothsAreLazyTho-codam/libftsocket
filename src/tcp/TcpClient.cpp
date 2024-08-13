#include "tcp/TcpClient.hpp"

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

	//this->_ip = Utils::addrintoipv4(addr.sin_addr);
	this->_ip = 
	this->_port = addr.sin_port;
	this->setConnected(true);
}

TcpClient::~TcpClient()
{
	this->_buffer.clear();
}

TcpClient::TcpClient(const TcpClient &other)
{
	*this = other;
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

/**
 * @brief Connects the TcpClient to a specified host and port.
 *
 * This function establishes a TCP connection to the specified host and port.
 * If the TcpClient is already connected, the function returns immediately.
 *
 * @param host The host to connect to.
 * @param port The port to connect to.
 * @return Returns 1 if the connection is successful, otherwise returns an error code.
 */
int TcpClient::connect(const char *host, const char *port)
{
	if (this->isConnected())
		return (1);

	if (Socket::open(host, port, CLIENT) < 0)
		return (-1);

	if (::connect(	this->fd.fd,
					this->addrinfo->ai_addr,
					this->addrinfo->ai_addrlen) < 0)
		return (-1);

	this->setConnected(true);
	return (1);
}

//TODO Implement sending feature if TcpClient is implemented manually,
void TcpClient::send(std::string buff)
{
	if (this->isClosed())
		return;

	if (buff.find("\r\n") == std::string::npos)
		buff += "\r\n";

	this->_buffer += buff;
}


/**
 * @brief Reads data from the TCP client socket.
 *
 * This function reads data from the TCP client socket and returns it as a vector of characters.
 * It reads data in chunks of 1024 bytes until there is no more data to read or an error occurs.
 * If the socket is closed, an exception of type std::invalid_argument is thrown.
 * If the connection is reset by the peer, an exception of type std::invalid_argument is thrown.
 * If the recv() function fails, an exception of type std::invalid_argument is thrown.
 *
 * @return A vector of characters containing the data read from the socket.
 *
 * @throws std::invalid_argument If the socket is closed, the connection is reset by the peer, or the recv() function fails.
 */
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

/**
 * Reads a string from the TCP connection.
 * 
 * @return The string read from the TCP connection.
 * @throws std::invalid_argument if no data was received by the peer.
 */
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


std::string TcpClient::getBuffer() const
{
	return (this->_buffer);
}

int32_t TcpClient::getBufferLength() const
{
	return (this->_buffer.length());
}

void	TcpClient::resetBuffer(void)
{
	this->_buffer.clear();
}