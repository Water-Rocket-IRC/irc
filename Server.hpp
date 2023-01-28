#pragma once

#include "Receiver_working.hpp"

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

static bool	_port_checker(std::string &str)
{
	if (str.empty())
		return (true);
	for (size_t i = 0; i < str.length(); ++i)
		if (!std::isdigit(str.at(i)))
			return (true);
	return (false);
}

static bool _pw_checker(std::string &str)
{
	if (str.empty() || str.length() > 16)
		return (true);
	for (size_t i = 0; i < str.length(); ++i)
		if (std::isspace(str.at(i)) || !std::isprint(str.at(i)))
			return (true);
	return (false);
}

Server::Server(std::string port, std::string password)
{
	if (_port_checker(port) && _pw_checker(password))
		exit_with_perror("input port invaild");
	port_ = atoi(port.c_str());
	if (port_ > 65535)
		exit_with_perror("Port number is out of range");
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