#ifndef SERVER_HPP
# define SERVER_HPP

#include "Udata.hpp"
#include <iostream>

// class Reciever;

class Server
{
	private:
		Udata			serv_udata_;
		std::string		password_;
		uintptr_t		port_;

	public:
		Server(const std::string& port, const std::string& password);
		~Server();

		void	start();
		Udata&	get_server_udata(void);
};

#endif