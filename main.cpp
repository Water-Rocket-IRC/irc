#include "Server.hpp"

int main(int argc, char **argv)
{
	if (argc != 3)
	{
		//에러
		system("clear");
		std::cerr << RED << "err: Wrong Arguments" << RESET << std::endl;
		exit(EXIT_FAILURE);
	}
	std::string	port(argv[1]);
	std::string password(argv[2]);
	Server		server(port, password);

	server.start();
}