#include "Server.hpp"
#include "Receiver.hpp"
#include "Udata.hpp"

static bool	_port_checker(const std::string &str)
{
	if (str.empty())
	{
		return (true);
	}
	for (std::size_t i = 0; i < str.length(); ++i)
	{
		if (!std::isdigit(str.at(i)))
			return (true);
	}
	return (false);
}

static bool _pw_checker(const std::string &str)
{
	if (str.empty() || str.length() > 16)
	{
		return (true);
	}
	for (std::size_t i = 0; i < str.length(); ++i)
	{
		if (std::isspace(str.at(i)) || !std::isprint(str.at(i)))
		{
			return (true);
		}
	}
	return (false);
}

Server::Server(const std::string& port, const std::string& password)
{
	if (_port_checker(port) || _pw_checker(password))
	{
		exit_with_perror("input port invaild");
	}
	port_ = static_cast<uintptr_t>(atoi(port.c_str()));
	if (port_ > 65535)
	{
		exit_with_perror("Port number is out of range");
	}
	password_ = password;
}

Server::~Server()
{
	
}

Udata&	Server::get_server_udata(void)
{
	return serv_udata_;
}

void Server::start()
{
	Receiver	receiver(get_server_udata(), port_, password_);

	receiver.start();
}
