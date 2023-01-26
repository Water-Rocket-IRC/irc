#pragma once

#include "Receiver.hpp"

class Server
{
	private:
		int	port_;
		std::string	password_;
	public:
		Server(std::string port, std::string password);
		~Server();
		void start();

};

static bool	_int_checker(std::string str)
{
	if (str.empty())
		return (true);
	for (size_t i = 0; i < str.length(); ++i)
		if (!std::isdigit(str.at(i)))
			return (true);
	return (false);
}

Server::Server(std::string port, std::string password)
{
	if (_int_checker(port))
		exit_with_perror("input port invaild");
	port_ = stoi(port);
	password_ = password;
}

Server::~Server()
{
	
}

void Server::start()
{
	Receiver receiver(port_);

	receiver.init();
	receiver.start();
}