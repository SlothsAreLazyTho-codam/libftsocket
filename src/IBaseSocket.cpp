#include "IBaseSocket.hpp"

#include "macros.h"

IBaseSocket::IBaseSocket():
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

IBaseSocket::~IBaseSocket()
{
	this->setConnected(false);
	if (this->addrinfo)
		::freeaddrinfo(this->addrinfo);
	if (this->fd.fd > 3)
		::close(this->fd.fd);
	LOG_DEBUG("Base socket deconstructed");
}

int IBaseSocket::open(const char *addr, const char *port, int type)
{
	if (this->fd.fd > STDERR_FILENO)
		return (this->fd.fd);

	this->_ip = addr;
	this->_port = port;
	
	NOT_BELOW_ZERO(getaddrinfo(addr, port, &_hints, &addrinfo));
	
	this->fd.fd = socket(
		this->addrinfo->ai_family,
		this->addrinfo->ai_socktype,
		this->addrinfo->ai_protocol
	);
	
	NOT_BELOW_ZERO(this->fd.fd);
	
	if (type == SERVER)
		NOT_BELOW_ZERO(fcntl(this->fd.fd, F_SETFL, O_NONBLOCK));

	return (this->fd.fd);
}

void IBaseSocket::close(void)
{
	if (this->isClosed())
		return;
	this->setConnected(false);
}

void IBaseSocket::setConnected(int open)
{
	if (open != 0 && open != 1)
		return;
	this->_open = open;
}

int IBaseSocket::isConnected(void) const
{
	return (this->_open);
}

int IBaseSocket::isClosed(void) const
{
	return (!this->_open);
}

int IBaseSocket::getSocket(void) const
{
	return (this->fd.fd);
}

struct addrinfo* IBaseSocket::getAddress(void) const
{
	return (this->addrinfo);
}

struct pollfd IBaseSocket::getFileDescriptor() const
{
	return (this->fd);
}

const std::string& IBaseSocket::getHost() const
{
	return (this->_ip);
}

const std::string& IBaseSocket::getPort() const
{
	return (this->_port);
}
