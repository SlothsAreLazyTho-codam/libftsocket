#include "tcp/TcpClient.hpp"

TcpClient::TcpClient(int fd, sockaddr_in addr):
	Socket(
		"client_stack",
		{ fd, POLLIN | POLLOUT | POLLHUP },
		addr),
	_connected(true)
{
	std::cout << "TcpClient constructor called" << std::endl;
}

TcpClient::TcpClient(pollfd fd, sockaddr_in addr): Socket("client_stack", fd, addr),
	_connected(true)
{
	std::cout << "TcpClient constructor called" << std::endl;
}


TcpClient::~TcpClient()
{
	this->flush();
	this->close();
	std::cout << "[Debug] client disconnected" << std::endl;
}

//Returns if connect went through, also returns true if the client is already connected
bool TcpClient::connect(std::string address, std::string port)
{
	if (this->isConnected())
		return (true);
	return (_connected);
}

void TcpClient::send(std::string arg)
{
	if (!this->isConnected())
		return;

	if (arg.find("\r\n") == std::string::npos)
		arg.append("\r\n");
	
	this->_buffer.append(arg);
}

std::vector<char> TcpClient::read()
{
	std::vector<char>	response(0);
	char				buffer[1024];
	int32_t				bytes_read = 1;

	if (this->isClosed())
	{
		throw std::invalid_argument("Cannot write to closed socket.");
	}

	while (bytes_read > 0)
	{
		bytes_read = recv(this->getFileDescriptor(), &buffer, 1024, 0);
		
		if (bytes_read == 0)
			throw std::invalid_argument("connection reset by peer.");

		if (bytes_read <= -1)
		{
			if (errno == EINTR)
				continue;
			else if (errno == EWOULDBLOCK || errno == EAGAIN)
				break;
			else
				throw std::invalid_argument("recv() failed");
		}
		response.insert(response.end(), buffer, buffer + bytes_read);
	}

	return (response);
}

void TcpClient::close()
{
	if (this->isClosed())
		return;
	this->_connected = false;
}

const bool TcpClient::isConnected() const { return (this->_connected); }
const bool TcpClient::isClosed() const { return (!this->_connected); }
const std::string &TcpClient::getBuffer() const { return (this->_buffer); }
const int TcpClient::getBufferLength() const { return (this->_buffer.size()); }
void TcpClient::flush() { this->_buffer.clear(); std::fill(_buffer.begin(), _buffer.end(), 0); }