#include "TcpServer.hpp"

#define LOG(x) std::cout << x << std::endl

int main()
{
	TcpServer	*server = new TcpServer();

	if (server->openup("0.0.0.0", "4242") < 0)
		return (LOG("Cannot open socket"), EXIT_FAILURE);

	//Add Events Handlers to make sure the TcpServer can do more than only socket handling XD

	server->loop();

	delete server;

	return (EXIT_SUCCESS);
}