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
		std::vector<struct kevent> events_;
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
		int		clientReadEventHandler(struct kevent &cur_event);
		int		clientWriteEventHandler(struct kevent &cur_event);

		user who_is_sender(struct kevent event);

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

void Receiver::start()
{
	while (true)
	{
		events_ = kq_.SetMonitor();
		for (size_t i(0); i < events_.size(); ++i)
		{
			struct kevent	cur_event = events_[i];	// event occur with new accept
			if (cur_event.ident == server_sock_)
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
				if (cur_event.filter == EVFILT_READ)
				{
					if (clientReadEventHandler(cur_event))
						continue ;
				}
				else if (cur_event.filter == EVFILT_WRITE)
				{
					if (clientWriteEventHandler(cur_event))
						continue ;
				}
			}
		}
	}
}

int	Receiver::clientReadEventHandler(struct kevent &cur_event)
{
	char buffer[512];

	memset(buffer, 0, sizeof(buffer));
	int byte_received = recv(cur_event.ident, buffer, sizeof(buffer), 0);
	if (byte_received < 0)
	{
		std::cerr << "err: receiving data" << std::endl;
		return (1);
	}
	std::cout << "Received: " << buffer << std::endl;
	std::string			command(buffer, byte_received);
	std::stringstream	ss(command);
	std::string			line;

	while (std::getline(ss, line, '\n'))
	{
		std::stringstream	line_ss(line);
		std::string			command_type;
		
		line_ss >> command_type;
		if (command_type == "NICK")
		{
			Users.addnick(line_ss, cur_event);
		}
		else if (command_type == "USER") 
		{
			Users.adduser(line_ss, cur_event.ident);
		}
		else if (command_type == "PING")
		{
			std::string serv_add;

			line_ss >> serv_add;
			std::cout << "PING received" << std::endl;
			Users.print_all_user(); //for debug
			kq_.SetWrite(cur_event.ident, 0); // TODO: have to add udata
			// Sender::pong(cur_event.ident, serv_add); // MOVE TO WRITE PART
		}
		else if (command_type == "PRIVMSG")
		{
			std::string target, msg;
			line_ss >> target >> msg;

			//1. send한 user identify
			user sender = Users.search_user_event(cur_event[i]);

			//2. receive할 user find
			user receiver = Users.search_user_nick(target);

			//여기서 write로 바꿔버린 다음에 보내면 안되냐?
			// receive의 Event를 write
			if (receiver.client_sock_ == -433)
			{
				Sender::send_err(sender, receiver, msg);
			}
			else
			{
				Sender::privmsg(sender, receiver, msg);
			}
			//Read로 바꿈 되잖아 
		}
	}
	return (0);
}

int	Receiver::clientWriteEventHandler(struct kevent &cur_event)
{
	kq_.SetRead(cur_event.ident, 0); // TODO: have to add udata
	return (0);
}
