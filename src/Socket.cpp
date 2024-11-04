#include "Socket.hpp"

Socket::Socket(std::string stack, const std::string &host, const std::string &port) : _host(host), _port(port),
																					  _stack(stack), _addrinfo(nullptr),
																					  _pollfd({-1, POLLIN | POLLOUT | POLLHUP, 0})
{
	// Setting hints assuming the socket has not been opened yet
	memset(&_hints, 0, sizeof(addrinfo));
	_hints.ai_flags = AI_ALL;		  /* Localhost address */
	_hints.ai_family = AF_INET;		  /* TCP Family */
	_hints.ai_socktype = SOCK_STREAM; /* TCP Stream */
}

Socket::Socket(std::string stack, int fd, sockaddr_in info) : _stack(stack), _addrinfo(nullptr),
															  _pollfd({fd, 0, 0}),
															  _host(inet_ntoa(info.sin_addr))
{
	memset(&_hints, 0, sizeof(addrinfo));
}

Socket::Socket(std::string stack, pollfd fd, sockaddr_in info) : _stack(stack), _addrinfo(nullptr),
																 _pollfd(fd),
																 _host(inet_ntoa(info.sin_addr))
{
	memset(&_hints, 0, sizeof(addrinfo));
}

Socket::~Socket()
{
	if (this->_addrinfo)
		::freeaddrinfo(this->_addrinfo);
	if (this->_pollfd.fd > STDERR_FILENO)
		::close(this->_pollfd.fd);
}

// Basically opens a socket pipe.
bool Socket::open()
{
	if (this->_pollfd.fd > STDERR_FILENO)
		return (true);

	if (getaddrinfo(_host.c_str(), _port.c_str(), &_hints, &_addrinfo) < 0)
		return (false);

	this->_pollfd.fd = socket(
		this->_addrinfo->ai_family,
		this->_addrinfo->ai_socktype,
		this->_addrinfo->ai_protocol);

	if (this->_pollfd.fd < 0)
		return (false);

	if (fcntl(this->_pollfd.fd, F_SETFL, O_NONBLOCK) < 0)
		return (false);

	return (this->_pollfd.fd > 1);
}

const std::string &Socket::getStack() const
{
	return (this->_stack);
}

const int Socket::getFileDescriptor() const
{
	return (this->_pollfd.fd);
}

const std::string &Socket::getHost() const
{
	return (this->_host);
}

const std::string &Socket::getPort() const
{
	return (this->_port);
}

//  * Protected functions * //
pollfd &Socket::getPollFiledescriptor()
{
	return (this->_pollfd);
}