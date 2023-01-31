#pragma once

#include "KeventHandler.hpp"
#include "Users.hpp"

#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/event.h>

#include <sys/types.h>
#include <netdb.h>

class Receiver
{
	private:
		KeventHandler		kq_;
		std::vector<Udata>	udata_;
		sockaddr_in			server_addr_;
		std::string			port_;
		std::string			password_;
		int					server_sock_;
		int					client_sock_;

	public:
		Users	Users;
		Receiver(int port);
		~Receiver();
		void	initSocket(int &port);
		void	bindSocket();
		void 	start();
		int		clientReadEventHandler(struct kevent &cur_event);
		int		parser(struct kevent &cur_event, std::stringstream &ss, std::string &line);
		int		clientWriteEventHandler(struct kevent &cur_event);
};


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
	{
		exit_with_perror("err: Socket Creating Fail");
	}
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
	{
		exit_with_perror("err: Socket Binding Fail");
	}
	if (listen(server_sock_, 5) < 0)
	{
		std::cerr << "error" << std::endl;
	} // TODO: Have to arrange 5 (Max queue)

	kq_.set_read(server_sock_);
}

void Receiver::start()
{
	while (true)
	{
		std::vector<struct kevent>	events = kq_.set_monitor();
		for (size_t i(0); i < events.size(); ++i)
		{
			struct kevent	cur_event = events[i];	// event occur with new accept
			if (cur_event.ident == server_sock_)
			{
				client_sock_ = accept(server_sock_, NULL, NULL);
				if (client_sock_ < 0)
				{
					std::cerr << "err: accepting connection fail" << std::endl;
					continue ;
				}
				kq_.set_read(client_sock_);
			}
			else	// event occur with users
			{
				if (cur_event.filter == EVFILT_READ)
				{
					if (clientReadEventHandler(cur_event))
					{
						continue ;
					}
				}
				else if (cur_event.filter == EVFILT_WRITE)
				{
					if (clientWriteEventHandler(cur_event))
					{
						int	tmp_fd = cur_event.ident;
						kq_.delete_event(cur_event);
						kq_.set_read(tmp_fd);
					}
				}
			}
		}
	}
}

int	Receiver::clientReadEventHandler(struct kevent &cur_event)
{
	char buffer[512];


	memset(buffer, 0, sizeof(buffer));
	if (cur_event.flags & EV_EOF)
	{
		std::cout << "sock was fucked!" << std::endl;
		kq_.delete_event(cur_event);
		// TODO: delete User
		return (1);
	}
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

	parser(cur_event, ss, line);

	return (0);
}

int	Receiver::parser(struct kevent &cur_event, std::stringstream &ss, std::string &line)
{
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
			Udata	tmp = Users.adduser(line_ss, cur_event.ident);
			udata_.push_back(tmp);
			kq_.set_write(cur_event.ident);
		}
		else if (command_type == "PING")
		{
			std::string serv_addr;
			line_ss >> serv_addr;

			Udata	tmp = Sender::pong(cur_event.ident, serv_addr);
			udata_.push_back(tmp);
			kq_.set_write(cur_event.ident);
		}
		// else if (command_type == "PRIVMSG")
		// {
		// 	std::string target, msg;
		// 	line_ss >> target >> msg;

		// 	//1. send한 user identify
		// 	user sender = Users.search_user_event(cur_event[i]);

		// 	//2. receive할 user find
		// 	user receiver = Users.search_user_nick(target);

		// 	//여기서 write로 바꿔버린 다음에 보내면 안되냐?
		// 	// receive의 Event를 write
		// 	if (receiver.client_sock_ == -433)
		// 	{
		// 		Sender::send_err(sender, receiver, msg);
		// 	}
		// 	else
		// 	{
		// 		Sender::privmsg(sender, receiver, msg);
		// 	}
		// 	//Read로 바꿈 되잖아 
	}
}

int	Receiver::clientWriteEventHandler(struct kevent &cur_event)
{
	if (udata_.size())
	{
		Udata	tmp;
		int		i;

		bzero(&tmp, sizeof(tmp));
		for (i = 0; i < udata_.size(); ++i)
		{
			if (udata_[i].sock_fd == cur_event.ident)
			{
				tmp = udata_[i];
				break ;
			}
		}
		std::cout << "socket: " << tmp.sock_fd << " msg: " << tmp.msg << std::endl;
		send(cur_event.ident, tmp.msg.c_str(), tmp.msg.length(), 0);
		udata_.erase(udata_.begin() + i);
		return (1);
	}
	return (0);
}
