#include "Server.hpp"
#include <signal.h>

static void	_sigint_handler(int signum)
{
	if (signum == SIGINT)
	{
		std::cout << BOLDCYAN << "Server Closed" << RESET << std::endl;
		Server::server_ptr_->server_sigint();
		exit(0);
	}
}

int main(int argc, char **argv)
{
	signal(SIGINT, _sigint_handler);
	system("clear");
	if (argc != 3)
	{
		std::cerr << RED << "err: Wrong Arguments" << RESET << std::endl;
		exit(EXIT_FAILURE);
	}
	std::string		port(argv[1]);
	std::string 	password(argv[2]);
	Server			server(port, password);
	Server::server_ptr_ = &server;
	server.start();
}
