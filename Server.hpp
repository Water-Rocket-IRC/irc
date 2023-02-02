#ifndef SERVER_HPP
# define SERVER_HPP

#include <iostream>

// class Reciever;

class Server
{
	private:
		int	port_;
		std::string	password_;
		// std::map<uintptr_t sock, std::string msg>;
	public:
		Server(std::string port, std::string password);
		~Server();
		void start();

};
#endif