#include "TcpClient.hpp"

TcpClient	g_Client;

void signal_c(int cod)
{
	(void)cod;
	g_Client.close();
}

void* connect_threaded_client(void *a)
{
	TcpClient _client;

	if (_client.connect("127.0.0.1", "4242") < 0)
		return (std::cout << "Cannot connect\n", nullptr);

	_client.send("Hello World");

	while (_client.isConnected());
	return (nullptr);
}

int main()
{
	int			amount = 100;
	pthread_t	thread_id[amount];
	for (int i = 0; i < amount; i++)
	{
		pthread_create(&thread_id[i], nullptr, connect_threaded_client, nullptr);
	}

	for (int i = 0; i < amount; i++)
	{
		pthread_join(thread_id[i], nullptr);
	}

	return (EXIT_SUCCESS);
}