#include "Socket.hpp"
#include "macros.h"

/**
 * @brief Default constructor for the Socket class.
 * 
 * This constructor initializes the Socket object with default values.
 * It sets the file descriptor to -1, the events to POLLIN, and the revents to 0.
 * It also sets the ai_flags to AI_ALL, ai_family to AF_INET, and ai_socktype to SOCK_STREAM.
 * 
 * @note This constructor does not create a socket connection.
 * 
 * @see Socket
 */
Socket::Socket():
	addrinfo(nullptr),
	_open(0)
{
	this->fd.fd = -1;
	this->fd.events = POLLIN;
	this->fd.revents = 0;

	bzero(&this->_hints, sizeof(this->_hints));
	_hints.ai_flags = AI_ALL; /* Localhost address */
	_hints.ai_family = AF_INET; /* TCP Family */
	_hints.ai_socktype = SOCK_STREAM; /* TCP Stream */
	LOG_DEBUG("Base socket constructed");
}

/**
 * @brief Destructor for the Socket class.
 * 
 * This destructor is responsible for cleaning up the resources used by the Socket object.
 * It sets the connection status to false, frees the addrinfo structure if it exists, and closes the file descriptor if it is valid.
 * Additionally, it logs a debug message indicating that the base socket has been deconstructed.
 */
Socket::~Socket()
{
	this->setConnected(false);
	if (this->addrinfo)
		::freeaddrinfo(this->addrinfo);
	if (this->fd.fd > 3)
		::close(this->fd.fd);
	LOG_DEBUG("Base socket deconstructed");
}

/**
 * Opens a socket connection.
 *
 * @param addr The address to connect to.
 * @param port The port to connect to.
 * @param type The type of socket (SERVER or CLIENT).
 * @return The file descriptor of the opened socket.
 */
int Socket::open(const char *addr, const char *port, int type)
{
	if (this->fd.fd > STDERR_FILENO)
		return (this->fd.fd);

	this->_ip = addr;
	this->_port = port;
	
	if (getaddrinfo(addr, port, &_hints, &addrinfo) < 0)
		return (-1);
	
	this->fd.fd = socket(
		this->addrinfo->ai_family,
		this->addrinfo->ai_socktype,
		this->addrinfo->ai_protocol
	);
	
	if (this->fd.fd < 0)
		return (-1);
	
	if (type == SERVER)
	{
		if (fcntl(this->fd.fd, F_SETFL, O_NONBLOCK) < 0)
			return (-1);
	}

	return (this->fd.fd);
}

/**
 * @brief Closes the socket connection.
 * 
 * This function closes the socket connection if it is not already closed.
 * It sets the connected flag to false.
 */
void Socket::close(void)
{
	if (this->isClosed())
		return;
	this->setConnected(false);
	::close(this->fd.fd);
}

void Socket::setConnected(int open)
{
	if (open != 0 && open != 1)
		return;
	this->_open = open;
}

int Socket::isConnected(void)
{
	return (this->_open);
}

int Socket::isClosed(void)
{
	return (!this->_open);
}

int Socket::getSocket(void)
{
	return (this->fd.fd);
}

struct addrinfo* Socket::getAddress(void)
{
	return (this->addrinfo);
}

struct pollfd Socket::getFileDescriptor()
{
	return (this->fd);
}

const std::string& Socket::getHost() const
{
	return (this->_ip);
}

const std::string& Socket::getPort() const
{
	return (this->_port);
}
