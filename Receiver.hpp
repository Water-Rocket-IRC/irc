#pragma once

#include "Users.hpp"
#include "Sender.hpp"

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

enum 
{
	PONG,
	WELCOME_MESSAGE,
	PRIVMSG,
	SEND_ERR
};

struct event_info
{
	int command;
	user sender;
	user receiver;
	std::string msg;
};

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
		std::vector<struct kevent> events_;
		struct kevent		tmp_;
		struct kevent		change_list_;
		sockaddr_in		server_addr_;
		sockaddr_in		client_addr_;
		int				client_addr_size_;
		int				client_sock_;
		int				server_sock_;
		int				kq_;
		int				ret_;
		std::string		port_;
		std::string		password_;

	public:
		Users	Users;
		Receiver(int port);
		~Receiver();
		void	init();
		void 	start();

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

void Receiver::init()
{
	// socket bind
	if (bind(server_sock_, (sockaddr *) &server_addr_, sizeof(server_addr_)) < 0)
		exit_with_perror("err: Socket Binding Fail");
		// throw SocketBindFail();

	listen(server_sock_, 5); // TODO: Have to arrange 5 (Max queue)
	kq_ = kqueue();
	if (kq_ < 0)
		exit_with_perror("err: Kqueue Creating Fail");
		// throw KqueueCreateFail();

	EV_SET(&change_list_, server_sock_, EVFILT_READ, EV_ADD, 0, 0, 0);
	ret_ = kevent(kq_, &change_list_, 1, NULL, 0, NULL);
	if (ret_ < 0)
		std::cerr << "err: adding server socket to kqueue" << std::endl;
}


void Receiver::start()
{
	char		buffer[512];

	while (1)
	{
		struct kevent events[10];
		struct timespec timeout;
		timeout.tv_sec = 1000 / 1000;
		timeout.tv_nsec = (1000 % 1000) * 1000000;
		int num_events = kevent(kq_, NULL, 0, events, 10, &timeout);
		if (num_events < 0)
		{
			std::cerr << "Error Waiting" << std::endl;
			continue ;
		}

		for (int i(0); i < num_events; ++i)
		{
			int	fd = (int) events[i].ident;
			if (fd == server_sock_)
			{
				client_sock_ = accept(server_sock_, (sockaddr *) &client_addr_, (socklen_t*) &client_addr_size_);
				if (client_sock_ < 0)
				{
					std::cerr << "err: accepting connection fail" << std::endl;
					continue ;
				}
				EV_SET(&change_list_, client_sock_, EVFILT_READ, EV_ADD, 0, 0, 0);
				ret_ = kevent(kq_, &change_list_, 1, NULL, 0, NULL);
				if (ret_ < 0)
				{
					std::cerr << "err: Adding client socket to kqueue fail" << std::endl;
					continue;
				}
			}
			else
			{
				memset(buffer, 0, sizeof(buffer));
				int byte_received = recv(client_sock_, buffer, sizeof(buffer), 0);
				if (byte_received < 0)
				{
					std::cerr << "Error receiving data" << std::endl;
					continue ;
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
						Users.addnick(line_ss, events[i]);
					}
					else if (command_type == "USER") 
					{
						Users.adduser(line_ss, events[i].ident);
					}
					else if (command_type == "PING")
					{
						std::string serv_add;

						line_ss >> serv_add;
						std::cout << "PING received" << std::endl;
						Users.print_all_user(); //for debug
						Sender::pong(events[i].ident, serv_add);
					}
					else if (command_type == "PRIVMSG")
					{
						std::string target, msg;
						line_ss >> target >> msg;

						//1. send한 user identify
						user sender = Users.search_user_event(events[i]);

						//2. receive할 user find
						user receiver = Users.search_user_nick(target);

						//
						if (receiver.client_sock_ == -433)
						{

							Sender::send_err(sender, msg);
						}
						else
						{
							Sender::privmsg(sender, receiver, msg);
						}
					}
				}
			}



		while (true) {
        // Wait for events
        int nev = kevent(kq_, NULL, 0, &tmp_, 1, NULL);

        for (int i = 0; i < nev; i++) {
			int	fd = (int) tmp_.ident;
            if (fd == server_sock_) {
				// Accept new client
				client_sock_ = accept(server_sock_, (sockaddr *) &client_addr_, (socklen_t*) &client_addr_size_);
				EV_SET(&change_list_, client_sock_, EVFILT_READ, EV_ADD, 0, 0, 0);
				ret_ = kevent(kq_, &change_list_, 1, NULL, 0, NULL);
				if (ret_ < 0)
				{
					std::cerr << "err: Adding client socket to kqueue fail" << std::endl;
					continue;
				}
				events_.push_back(change_list_);
            }
            else {
				for (int j = 0; j < events_.size(); j++) {
				    if (events_[j].ident == tmp_.ident) {
				        if (events_[j].filter == EVFILT_READ) {
							memset(buffer, 0, sizeof(buffer));
							int byte_received = recv(client_sock_, buffer, sizeof(buffer), 0);
							if (byte_received < 0)
							{
								std::cerr << "Error receiving data" << std::endl;
								continue ;
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
									Users.addnick(line_ss, events[i].ident);
								}
								else if (command_type == "USER") 
								{
									Users.adduser(line_ss, events[i].ident);
								}
								else if (command_type == "PING")
								{
									std::string serv_add;

									line_ss >> serv_add;
									std::cout << "PING received" << std::endl;
									Users.print_all_user(); //for debug
									/////////////
									Sender::pong(events[i].ident, serv_add);
								}
								else if (command_type == "PRIVMSG")
								{
									std::string target, msg;
									line_ss >> target >> msg;

									//1. send한 user identify
									user sender = Users.search_user_event(events[i]);

									//2. receive할 user find
									user receiver = Users.search_user_nick(target);

									//여기서 write로 바꿔버린 다음에 보내면 안되냐?
									// receive의 Event를 write
									if (events_[j].filter == EVFILT_WRITE) 
									{
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
							}
							}
				        break;
				    }
				}
            }
        }
    }
		
		
//====================================

		}
		
	}
}

