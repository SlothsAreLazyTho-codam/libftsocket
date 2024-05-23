#include "TcpServer.hpp"
#include "TcpClient.hpp"
#include "Utils.hpp"

#define LOG(x) std::cout << x << std::endl

void	*parse_messages(TcpClient *c, const std::string& message)
{
	LOG("Incoming: " << message);
	return nullptr;
}

int main()
{
	TcpServer	*server = new TcpServer();

	if (server->openup("0.0.0.0", "4242") < 0)
		return (LOG("Cannot open socket"), EXIT_FAILURE);

	//Add Events Handlers to make sure the TcpServer can do more than only socket handling XD

	server->onMessage = parse_messages;

	server->loop();

	delete server;

	return (EXIT_SUCCESS);
}