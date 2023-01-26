#include "Server.hpp"

int main(int argc, char **argv)
{
	std::string	port(argv[1]);
	std::string password(argv[2]);
	Server		server(port, password);

	if (argc != 3)
	{
		//에러
	}
	server.start();
}