#include "tcp/TcpClient.hpp"

TcpClient::TcpClient(int fd, sockaddr_in addr):
	Socket("client_stack", {fd, POLLIN | POLLOUT | POLLHUP}, addr),
	_connected(true), _buffer(1024)
{ }

TcpClient::TcpClient(pollfd fd, sockaddr_in addr):
	Socket("client_stack", fd, addr),
	_connected(true), _buffer(1024)
{ }

TcpClient::~TcpClient()
{
	this->flush();
	this->close();
}

// Returns if connect went through, also returns true if the client is already connected
bool TcpClient::connect(std::string address, std::string port)
{
	if (this->isConnected())
		return (true);
	return (_connected);
}

// I chose to do buffer instead of sending directly because TcpClient#connect doesn't work yet.
void TcpClient::send(std::string &arg)
{
	if (!this->isConnected())
		return;

	if (arg.find("\r\n") == std::string::npos)
		arg.append("\r\n");

	this->_buffer.insert(_buffer.end(), arg.begin(), arg.end());
}

std::vector<char> TcpClient::read()
{
	std::vector<char>	response(0);
	char				buffer[1024];
	int					bytes_read = 1;

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
const std::vector<char> &TcpClient::getBuffer() const { return (this->_buffer); }
const int TcpClient::getBufferLength() const { return (this->_buffer.size()); }

void TcpClient::flush()
{
	this->_buffer.clear();
}