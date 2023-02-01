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

//member 함수로 일단 넣지는 않았음
static void _print_title(const std::string& title)
{

	std::string  under_bar = "____";
	std::string  over_bar =  "‾‾‾‾";
	for (std::size_t i = 0; i < title.length(); ++i)
	{
		under_bar += "_";
		over_bar += "‾";
	}
	std::cout << BOLDCYAN << under_bar << std::endl
						  << "| " << title << " |" << std::endl
					 	  << over_bar << std::endl
			  << RESET << std::endl;
}

class Receiver
{
	private:
		KeventHandler		kq_;
		std::vector<Udata>	udata_;
		sockaddr_in			server_addr_;
		std::string			port_;
		std::string			password_;
		uintptr_t			server_sock_;
		uintptr_t			client_sock_;

		void				bindSocket_();
		void				initSocket_(int &port);
		int					clientReadEventHandler_(struct kevent &cur_event);
		void				parser_(struct kevent &cur_event, std::string& command);
		void				push_write_event_(Udata &tmp, struct kevent &cur_event);
		void				push_write_event_with_vector_(std::vector<Udata> &udata_events);
		int					clientWriteEventHandler_(struct kevent &cur_event);
		std::string			set_message_(std::string &msg, std::size_t start, std::size_t end);
	public:
		Users		Users;
		Channels	Channels;
		Receiver(int port);
		~Receiver();

		void 	start();
};




/*    Receiver Class     */
/// @brief Receiver 생성자
/// @param port 소켓을 생성할 포켓 번호
Receiver::Receiver(int port)
{
	initSocket_(port);
	bindSocket_();
}

void	Receiver::initSocket_(int &port)
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

void Receiver::bindSocket_()
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
		for (std::size_t i(0); i < events.size(); ++i)
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
					if (clientReadEventHandler_(cur_event))
					{
						continue ;
					}
				}
				else if (cur_event.filter == EVFILT_WRITE)
				{
					// TODO: LEAK CHECK
					if (clientWriteEventHandler_(cur_event))
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

int	Receiver::clientReadEventHandler_(struct kevent &cur_event)
{
	char buffer[1024];


	memset(buffer, 0, sizeof(buffer));
	// TODO: LEAK CHECK
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

	parser_(cur_event, command);
	return (0);
}

void	Receiver::parser_(struct kevent &cur_event, std::string &command)
{
	std::stringstream	ss(command);
	std::string			line;

	while (std::getline(ss, line, '\n'))
	{
		std::stringstream	line_ss(line);
		std::string			command_type;

		line_ss >> command_type;
		if (command_type == "NICK")
		{
			_print_title(command_type);

			//check_argument(2, )
			// NICK은 채널에 있을 때 모든 유저에게 NICK 변경되었음을 알리고, 로비에 있을 때는 본인에게만 알린다.
			std::string	argument[2];

			line_ss >> argument[0] >> argument[1];
			if (!argument[1].empty())
			{
				//argument 에러
			}
			Udata	tmp = Users.command_nick(argument[0], cur_event);
			if (tmp.msg.empty()) {
				push_write_event_(tmp, cur_event);
				continue ;
			}
			std::vector<Udata>	events;
			events.push_back(tmp);
			try
			{
				user& 	who = Users.search_user_by_ident(cur_event.ident);
				std::vector<Udata> tmp_events = Channels.nick_channel(who, tmp.msg);
				events.insert(events.end(), tmp_events.begin(), tmp_events.end());
				push_write_event_with_vector_(events);
				Users.change_nickname(who, argument[0]);
			}
			catch (const std::exception& e)
			{
				push_write_event_(tmp, cur_event);
			}
		}
		else if (command_type == "USER")
		{
			_print_title(command_type);

			Udata	tmp = Users.command_user(line_ss, cur_event.ident);
			push_write_event_(tmp, cur_event);
		}
		else if (command_type == "PING")
		{
			_print_title(command_type);

			std::string serv_addr;
			line_ss >> serv_addr;

			Udata	tmp = Sender::pong(cur_event.ident, serv_addr);
			push_write_event_(tmp, cur_event);
		}
		else if (command_type == "QUIT")
		{
			_print_title(command_type);

			Udata	tmp = Users.command_quit(line_ss, cur_event.ident); //벡터로 바꿔야함
			push_write_event_(tmp, cur_event);
		}
		else if (command_type == "PRIVMSG")
		{
			_print_title(command_type);

			std::string target, msg;
			line_ss >> target;
			
			try
			{
				std::size_t	pos = line.find(':');
				msg = set_message_(line, pos + 1, (line.length() - (pos + 2)));
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
				push_write_event_with_vector_(udata_events);
			}
			else
			{
				Udata	tmp = Users.command_privmsg(line_ss, line, cur_event.ident);
				push_write_event_(tmp, cur_event);
			}
		}
		else if (command_type == "NOTICE")
		{
			_print_title(command_type);

			std::string chan_name, msg;

			line_ss >> chan_name >> msg;
			if (msg[0] != ':')
			{
				//오류
			}
			else
			{
				try
				{
					user sender = Users.search_user_by_ident(cur_event.ident);
					std::size_t	pos = line.find(':');
					msg = set_message_(line, pos + 1, (line.length() - (pos + 2)));
					(msg.size() > 510) ? msg.resize(510) : msg.resize(msg.size());

					Udata	udata_ = Channels.channel_wall(sender, chan_name, msg);
					push_write_event_(udata_, cur_event);
				}
				catch (std::exception &e)
				{
					std::cout << "FUck" << std::endl;
				}
			}
		}
		else if (command_type == "WALL")
		{
			_print_title(command_type);

			std::string chan_name, msg;

			line_ss >> chan_name >> msg;
			if (msg[0] != ':')
			{
				//오류
			}
			else
			{
				try
				{
					user sender = Users.search_user_by_ident(cur_event.ident);
					std::size_t	pos = line.find(':');
					msg = set_message_(line, pos + 1, (line.length() - (pos + 2)));
					(msg.size() > 510) ? msg.resize(510) : msg.resize(msg.size());

					std::vector<Udata>	udata_events = Channels.channel_notice(sender, chan_name, msg);
					push_write_event_with_vector_(udata_events);
				}
				catch (std::exception &e)
				{
					std::cout << "FUck" << std::endl;
				}
			}
		}
		else if (command_type == "JOIN")
		{
			_print_title(command_type);

			std::string	chan_name, error;


			line_ss >> chan_name >> error;
			if (!error.empty())
			{
				//error 나중에 nc로 넣어보기
			}
			else
			{
				user chan_user = Users.search_user_by_ident(cur_event.ident);
				std::vector<Udata>	udata_events = Channels.join_channel(chan_user, chan_name);
				push_write_event_with_vector_(udata_events);
			}

		}
		else if (command_type == "PART")
		{
			_print_title(command_type);

			std::string chan_name, msg;

			line_ss >> chan_name >> msg; 
			user leaver = Users.search_user_by_ident(cur_event.ident);
			std::vector<Udata>	udata_events = Channels.leave_channel(leaver, chan_name, msg);
			push_write_event_with_vector_(udata_events);
		}
		else if (command_type == "TOPIC")
		{
			_print_title(command_type);

			std::string chan_name, topic;
			line_ss >> chan_name >> topic;

			try
			{
				user sender = Users.search_user_by_ident(cur_event.ident);
				std::size_t	pos = line.find(':');
				topic = set_message_(line, pos + 1, (line.length() - (pos + 2)));
				(topic.size() > 510) ? topic.resize(510) : topic.resize(topic.size());

				std::vector<Udata>	udata_events = Channels.set_topic(sender, chan_name, topic);
				push_write_event_with_vector_(udata_events);
			}
			catch (std::exception &e)
			{
				std::cout << "FUck" << std::endl;
			}
		}
		else if (command_type == "KICK")
		{
			_print_title(command_type);

			std::string chan_name, target_name, msg;

			line_ss >> chan_name >> target_name;
			try
			{
				user kicker = Users.search_user_by_ident(cur_event.ident);
				user target = Users.search_user_by_nick(target_name); 
				std::size_t	pos = line.find(':');
				msg = set_message_(line, pos + 1, (line.length() - (pos + 2)));
				(msg.size() > 510) ? msg.resize(510) : msg.resize(msg.size());
				std::vector<Udata>	udata_events = Channels.kick_channel(kicker, target, chan_name, msg);
				push_write_event_with_vector_(udata_events);
			}
			catch (std::exception &e)
			{
				std::cout << "FUck" << std::endl;
			}
		}
	}
}

std::string	Receiver::set_message_(std::string &msg, size_t start, size_t end)
{
	std::string	ret = msg.substr(start, end);
	return ret;
}

void	Receiver::push_write_event_(Udata& tmp, struct kevent &cur_event)
{
	if (tmp.msg.size())
	{
		udata_.push_back(tmp);
		kq_.set_write(cur_event.ident);
	}
}

void	Receiver::push_write_event_with_vector_(std::vector<Udata>& udata_events)
{
	for (std::size_t i(0); i < udata_events.size(); ++i)
	{
		udata_.push_back(udata_events[i]);
		kq_.set_write(udata_events[i].sock_fd);
	}
}

// void	Receiver::push_write_event_with_vector_(std::size_t pos, std::vector<Udata>& udata_events)
// {
// 	for (std::size_t i(0); i < udata_events.size(); ++i)
// 	{
// 		udata_.push_back(udata_events[i]);
// 		kq_.set_write(udata_events[i].sock_fd);
// 	}
// }

int	Receiver::clientWriteEventHandler_(struct kevent &cur_event)
{
	if (udata_.size())
	{
		for (std::size_t i(0); i < udata_.size(); ++i)
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
