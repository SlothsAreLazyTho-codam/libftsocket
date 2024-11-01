#include "Socket.hpp"

Socket::Socket(std::string stack, const std::string host, const std::string port):
	_host(host), _port(port),
	_stack(stack), _addrinfo(nullptr),
	_pollfd({ -1, POLLIN, 0 })
{
	_hints.ai_flags = AI_ALL; /* Localhost address */
	_hints.ai_family = AF_INET; /* TCP Family */
	_hints.ai_socktype = SOCK_STREAM; /* TCP Stream */
}

Socket::Socket(std::string stack, int fd, sockaddr_in info):
	_stack(stack), _addrinfo(nullptr),
	_pollfd({ fd, 0, 0 })
{
	/* Get and set address info from sockaddr_in */
}

Socket::Socket(std::string stack, pollfd fd, sockaddr_in info):
	_stack(stack), _addrinfo(nullptr),
	_pollfd(fd)
{
	/* Get and set address info from sockaddr_in */
}

Socket::~Socket()
{
	if (this->_addrinfo)
		::freeaddrinfo(this->_addrinfo);
	if (this->_pollfd.fd > STDERR_FILENO)
		::close(this->_pollfd.fd);
	std::cout << "[Info] Socket is closed" << std::endl;
}

//Basically opens a socket pipe.
bool Socket::open()
{
	const char *addr = this->_host.c_str();

	if (this->_pollfd.fd > STDERR_FILENO)
		return (true);
	
	if (getaddrinfo(addr, _port.c_str(), &_hints, &_addrinfo) < 0)
		return (false);
	
	this->_pollfd.fd = socket(
		this->_addrinfo->ai_family,
		this->_addrinfo->ai_socktype,
		this->_addrinfo->ai_protocol
	);
	
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

const std::string &Socket::getPort() const
{
	return (this->_port);
}

//Protected functions
pollfd &Socket::getPollFiledescriptor()
{
	return (this->_pollfd);
}