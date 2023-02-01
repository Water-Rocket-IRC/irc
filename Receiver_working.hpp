#pragma once

#include "KeventHandler.hpp"
#include "Users.hpp"
#include "Channel.hpp"
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
		Users		Users;
		Channels	Channels;
		Receiver(int port);
		~Receiver();
		void		initSocket(int &port);
		void		bindSocket();
		void 		start();
		int			clientReadEventHandler(struct kevent &cur_event);
		void		parser(struct kevent &cur_event, std::string& command);
		void		push_write_event(Udata &tmp, struct kevent &cur_event);
		void		push_write_event_with_vector(std::vector<Udata> &udata_events, Udata& tmp);
		int			clientWriteEventHandler(struct kevent &cur_event);
		std::string	set_message(std::string &msg, size_t start, size_t end);
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
					// TODO: LEAK CHECK
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
	char buffer[1024];


	memset(buffer, 0, sizeof(buffer));
	// TODO: LEAK CHECK
	if (cur_event.flags & EV_EOF)
	{
		std::cout << "sock was fucked!" << std::endl;
		Udata	*tmp = reinterpret_cast<Udata *>(cur_event.udata);
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

	parser(cur_event, command);
	return (0);
}

void	Receiver::parser(struct kevent &cur_event, std::string &command)
{
	std::stringstream	ss(command);
	std::string			line;

	while (std::getline(ss, line, '\n'))
	{
		std::stringstream	line_ss(line);
		std::string			command_type;
		Udata				tmp;
		
		line_ss >> command_type;
		if (command_type == "NICK")
		{
			tmp = Users.command_nick(line_ss, cur_event);
			push_write_event(tmp, cur_event);
		}
		else if (command_type == "USER") 
		{
			tmp = Users.command_user(line_ss, cur_event.ident);
			push_write_event(tmp, cur_event);
		}
		else if (command_type == "PING")
		{
			std::string serv_addr;
			line_ss >> serv_addr;

			tmp = Sender::pong(cur_event.ident, serv_addr);
			push_write_event(tmp, cur_event);
		}
		else if (command_type == "QUIT")
		{
			tmp = Users.command_quit(line_ss, cur_event.ident); //벡터로 바꿔야함
			push_write_event(tmp, cur_event);
		}
		else if (command_type == "PRIVMSG")
		{
			std::string target, msg;
			line_ss >> target;
			
			try
			{
				size_t	pos = line.find(':');
				msg = set_message(line, pos + 1, (line.length() - (pos + 2)));
				(msg.size() > 510) ? msg.resize(510) : msg.resize(msg.size());
				std::cout << "Mesaage size : " << msg.at(msg.size() - 1) << " aaa " << std::endl;

				std::cout << msg << std::endl;
				std::cout << target << std::endl;
			}
			catch (std::exception &e)
			{
				std::cout << "wow" << std::endl;
			}

			if (target.at(0) == '#')
			{
				user sender = Users.search_user_by_ident(cur_event.ident);

				std::vector<Udata>	udata_events = Channels.channel_msg(sender, target, msg);
				push_write_event_with_vector(udata_events, tmp);
			}
			else
			{
				tmp = Users.command_privmsg(line_ss, line, cur_event.ident);
				push_write_event(tmp, cur_event);
			}
		}
		else if (command_type == "NOTICE")
		{
			//notice in channel
			//std::string 	
		}
		//Channels
		// else if (command_type == "WALL")
		// {
		// 	tmp = Channels.channel_wall();
		// }
		else if (command_type == "JOIN")
		{
			std::string	chan_name;

			line_ss >> chan_name;
			user chan_user = Users.search_user_by_ident(cur_event.ident);
			std::vector<Udata>	udata_events = Channels.join_channel(chan_user, chan_name);
			push_write_event_with_vector(udata_events, tmp);
		}
		else if (command_type == "PART")
		{
			std::string chan_name, msg;

			line_ss >> chan_name, msg;
			user leaver = Users.search_user_by_ident(cur_event.ident);
			std::vector<Udata>	udata_events = Channels.leave_channel(leaver, chan_name, msg);
			push_write_event_with_vector(udata_events, tmp);
		}
		// else if (command_type == "TOPIC")
		// {
		// 	std::vector<Udata>	udata_events = Channels.
		// 	push_write_event_with_vector(udata_events, tmp);
		// }
		else if (command_type == "KICK")
		{
			std::string chan_name, target_name, msg;

			line_ss >> chan_name >> target_name;
			try
			{
				user kicker = Users.search_user_by_ident(cur_event.ident);
				user target = Users.search_user_by_nick(target_name); 
				size_t	pos = line.find(':');
				msg = set_message(line, pos + 1, (line.length() - (pos + 2)));
				(msg.size() > 510) ? msg.resize(510) : msg.resize(msg.size());
				std::vector<Udata>	udata_events = Channels.kick_channel(kicker, target, chan_name, msg);
				push_write_event_with_vector(udata_events, tmp);
			}
			catch (std::exception &e)
			{
				std::cout << "FUck" << std::endl;
			}
		}
	}
}

std::string	Receiver::set_message(std::string &msg, size_t start, size_t end)
{
	std::string	ret = msg.substr(start, end);
	return ret;
}

void	Receiver::push_write_event(Udata& tmp, struct kevent &cur_event)
{
	if (tmp.msg.size())
	{
		udata_.push_back(tmp);
		kq_.set_write(cur_event.ident);
	}
}

void	Receiver::push_write_event_with_vector(std::vector<Udata>& udata_events, Udata& tmp)
{
	for (int i(0); i < udata_events.size(); ++i)
	{
		tmp = udata_events[i];
		udata_.push_back(tmp);
		kq_.set_write(udata_events[i].sock_fd);
	}
}

int	Receiver::clientWriteEventHandler(struct kevent &cur_event)
{
	if (udata_.size())
	{
		int		i;

		for (i = 0; i < udata_.size(); ++i)
		{
			if (udata_[i].sock_fd == cur_event.ident)
			{
				std::cout << "socket: " << udata_[i].sock_fd << " msg: " << udata_[i].msg << std::endl;
				send(cur_event.ident, udata_[i].msg.c_str(), udata_[i].msg.length(), 0);
				udata_.erase(udata_.begin() + i);
			}
		}
		return (1);
	}
	return (0);
}
