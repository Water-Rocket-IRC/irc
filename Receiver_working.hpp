#pragma once

#include "Users.hpp"
#include "Sender.hpp"
#include "KeventHandler.hpp"

#include <iostream>
#include <string>
#include <cstring>
#include <vector>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/event.h>

class Receiver
{
	class SocketCreateFail : public std::exception
	{
		public :
			const char*	what() const throw();
	};
	class SocketBindFail : public std::exception
	{
		public :
			const char*	what() const throw();
	};
	class KqueueCreateFail : public std::exception
	{
		public :
			const char*	what() const throw();
	};

	private:
		KeventHandler	kq_;
		sockaddr_in		server_addr_;
		sockaddr_in		client_addr_;
		int				client_sock_;
		int				server_sock_;
		std::string		port_;
		std::string		password_;

	public:
		Users	Users;
		Receiver(int port);
		~Receiver();
		void	init();
		void	initSocket(int &port);
		void	bindSocket();
		void 	start();
		int		clientAccept(struct kevent &cur);

		user who_is_sender(struct kevent event)

};

/*    Throw Class     */
// const char*	Receiver::SocketCreateFail::what() const throw()
// {
// 	system("clear");
// 	return ("err: Socket creating fail");
// }

// const char*	Receiver::SocketBindFail::what() const throw()
// {
// 	system("clear");
// 	return ("err: Socket binding fail");
// }

// const char*	Receiver::KqueueCreateFail::what() const throw()
// {
// 	system("clear");
// 	return ("err: Kqueue creating fail");
// }

void exit_with_perror(const std::string& msg)
{
	system("clear");
	std::cerr << msg << std::endl;
	exit(EXIT_FAILURE);
}

/*    Receiver Class     */
/// @brief Receiver 생성자
/// @param port 소켓을 생성할 포켓 번호
Receiver::Receiver(int port)
{
	initSocket(port);
	bindSocket();
}

void	Receiver::initSocket(int &port)
{
	server_sock_ = socket(AF_INET, SOCK_STREAM, 0);
	if (server_sock_ < 0)
		exit_with_perror("err: Socket Creating Fail");
		// throw SocketCreateFail();
	server_addr_.sin_family = AF_INET;
	server_addr_.sin_port = htons(port);
	server_addr_.sin_addr.s_addr = INADDR_ANY;
}

Receiver::~Receiver()
{
}

void Receiver::bindSocket()
{
	// socket bind
	if (bind(server_sock_, (sockaddr *) &server_addr_, sizeof(server_addr_)) < 0)
		exit_with_perror("err: Socket Binding Fail");
		// throw SocketBindFail();

	listen(server_sock_, 5); // TODO: Have to arrange 5 (Max queue)

	kq_.SetRead(server_sock_, 0);
}

int	Receiver::clientAccept(struct kevent &cur)
{
	client_sock_ = accept(server_sock_, (sockaddr *) &client_addr_, (socklen_t*) &client_addr_size_);
	if (client_sock_ < 0)
	{
		std::cerr << "err: accepting connection fail" << std::endl;
		return (0);
	}
	kq_.SetRead(client_sock_, 0); // ADD udata or not
}	

void Receiver::start()
{
	while (true)
	{
		std::vector<struct kevent> events = kq_.SetMonitor();
		for (size_t i(0); i < events.size(); ++i)
		{
			struct kevent	cur = events[i];	// event occur with new accept
			if (cur.ident == server_sock_)
			{
				client_sock_ = accept(server_sock_, (sockaddr *) &client_addr_, (socklen_t*) sizeof(client_addr_));
				if (client_sock_ < 0)
				{
					std::cerr << "err: accepting connection fail" << std::endl;
					continue ;
				}
				kq_.SetRead(client_sock_, 0); // ADD udata or not
			}
			else	// event occur with users
			{
				
			}
		}
	}
}

