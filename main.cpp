#include "Server.hpp"

int main(int argc, char** argv)
{
	(void)argc;
	(void)argv;

	irc::Server server(6667, "password");

	server.startServer();

	return (0);
}