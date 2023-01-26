#ifndef SERVERHPP
#define SERVERHPP

#include "Receiver.hpp"

class Server
{
	private:

	public:
		Server();
		~Server();
		void start();
};

Server::Server()
{
	
}

Server::~Server()
{
	
}

void Server::start()
{
	Receiver receiver;

	receiver.init();
	receiver.start();
}

#endif