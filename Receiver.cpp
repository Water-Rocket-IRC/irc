#include "Receiver.hpp"
#include "Parser.hpp"

KeventHandler	Receiver::get_Kevent_Handler(void)
{
	return kq_;
}

// class Parser;
/*    Receiver Class     */
/// @brief Receiver 생성자
/// @param port 소켓을 생성할 포켓 번호
Receiver::Receiver(int port)
{
	init_socket_(port);
	bind_socket_();
}

Receiver::~Receiver()
{
	close(server_sock_);
}

void	Receiver::init_socket_(int &port)
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

void	Receiver::bind_socket_()
{
	// socket bind
	if (bind(server_sock_, (sockaddr *) &server_addr_, sizeof(server_addr_)) < 0)
	{
		exit_with_perror("err: Socket Binding Fail");
	}
	if (listen(server_sock_, 5) < 0)
	{
		std::cerr << "error" << std::endl;
	}
	// TODO: Have to arrange 5 (Max queue)
	kq_.set_read(server_sock_);
}

void	Receiver::start()
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
	Parser	parser;
	char	buffer[1024];


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
	std::cout << BOLDYELLOW << "Received: " << cur_event.ident << "\n" << buffer << RESET<< std::endl;
	std::string			command(buffer, byte_received);

	parser.command_parser(cur_event, command);
	return (0);
}

// void	Receiver::parser_(struct kevent &cur_event, std::string &command)
// {
// 	std::stringstream	ss(command);
// 	std::string			line;

// 	while (std::getline(ss, line, '\n'))
// 	{
// 		std::stringstream	line_ss(line);
// 		std::string			command_type;

// 		line_ss >> command_type;
// 		_print_title(command_type);
// 		if (command_type == "NICK")
// 		{
// 			//check_argument(2, )
// 			// NICK은 채널에 있을 때 모든 유저에게 NICK 변경되었음을 알리고, 로비에 있을 때는 본인에게만 알린다.
// 			std::string	argument[2];

// 			line_ss >> argument[0] >> argument[1];
// 			if (!argument[1].empty())
// 			{
// 				//argument 에러
// 			}
// 			Udata	tmp = Users.command_nick(argument[0], cur_event);
// 			if (tmp.msg.empty()) {
// 				push_write_event(tmp, cur_event);
// 				continue ;
// 			}
// 			std::vector<Udata>	events;
// 			events.push_back(tmp);
// 			try
// 			{
// 				user& 	who = Users.search_user_by_ident(cur_event.ident);
// 				std::vector<Udata> tmp_events = Channels.nick_channel(who, tmp.msg);
// 				events.insert(events.end(), tmp_events.begin(), tmp_events.end());
// 				push_write_event_with_vector(events);
// 			}
// 			catch (const std::exception& e)
// 			{
// 				push_write_event(tmp, cur_event);
// 			}
// 		}
// 		else if (command_type == "USER")
// 		{
// 			Udata	tmp = Users.command_user(line_ss, cur_event.ident);
// 			push_write_event(tmp, cur_event);
// 		}
// 		else if (command_type == "PING")
// 		{
// 			std::string serv_addr;
// 			line_ss >> serv_addr;

// 			Udata	tmp = Sender::pong(cur_event.ident, serv_addr);
// 			push_write_event(tmp, cur_event);
// 		}
// 		else if (command_type == "QUIT")
// 		{
// 			Udata	tmp = Users.command_quit(line_ss, cur_event.ident); //벡터로 바꿔야함
// 			push_write_event(tmp, cur_event);
// 		}
// 		else if (command_type == "PRIVMSG")
// 		{
// 			std::string target, msg;
// 			line_ss >> target;

// 			try
// 			{
// 				std::size_t	pos = line.find(':');
// 				msg = set_message_(line, pos + 1, (line.length() - (pos + 2)));
// 				(msg.size() > 510) ? msg.resize(510) : msg.resize(msg.size());
// 				std::cout << "Mesaage size : " << msg.at(msg.size() - 1) << " aaa " << std::endl;

// 				std::cout << msg << std::endl;
// 				std::cout << target << std::endl;
// 			}
// 			catch (std::exception &e)
// 			{
// 				std::cout << "wow" << std::endl;
// 			}

// 			if (target.at(0) == '#')
// 			{
// 				user sender = Users.search_user_by_ident(cur_event.ident);

// 				std::vector<Udata>	udata_events = Channels.channel_msg(sender, target, msg);
// 				push_write_event_with_vector(udata_events);
// 			}
// 			else
// 			{
// 				Udata	tmp = Users.command_privmsg(line_ss, line, cur_event.ident);
// 				push_write_event(tmp, cur_event);
// 			}
// 		}
// 		else if (command_type == "NOTICE")
// 		{
// 			std::string chan_name, msg;

// 			line_ss >> chan_name >> msg;
// 			if (msg[0] != ':')
// 			{
// 				//오류
// 			}
// 			else
// 			{
// 				try
// 				{
// 					user sender = Users.search_user_by_ident(cur_event.ident);
// 					std::size_t	pos = line.find(':');
// 					msg = set_message_(line, pos + 1, (line.length() - (pos + 2)));
// 					(msg.size() > 510) ? msg.resize(510) : msg.resize(msg.size());

// 					Udata	udata_ = Channels.channel_wall(sender, chan_name, msg);
// 					push_write_event(udata_, cur_event);
// 				}
// 				catch (std::exception &e)
// 				{
// 					std::cout << "FUck" << std::endl;
// 				}
// 			}
// 		}
// 		else if (command_type == "WALL")
// 		{
// 			std::string chan_name, msg;

// 			line_ss >> chan_name >> msg;
// 			if (msg[0] != ':')
// 			{
// 				//오류
// 			}
// 			else
// 			{
// 				try
// 				{
// 					user sender = Users.search_user_by_ident(cur_event.ident);

// 					std::size_t	pos = line.find(':');
// 					msg = set_message_(line, pos + 1, (line.length() - (pos + 2)));
// 					(msg.size() > 510) ? msg.resize(510) : msg.resize(msg.size());

// 					std::vector<Udata>	udata_events = Channels.channel_notice(sender, chan_name, msg);
// 					push_write_event_with_vector(udata_events);
// 				}
// 				catch (std::exception &e)
// 				{
// 					std::cout << "FUck" << std::endl;
// 				}
// 			}
// 		}
// 		else if (command_type == "JOIN")
// 		{
// 			std::string	chan_name, error;


// 			line_ss >> chan_name >> error;
	
// 			if (!error.empty())
// 			{
// 				//error 나중에 nc로 넣어보기
// 			}
// 			else
// 			{
// 				user chan_user = Users.search_user_by_ident(cur_event.ident);
// 				std::vector<Udata>	udata_events = Channels.join_channel(chan_user, chan_name);
// 				push_write_event_with_vector(udata_events);
// 			}

// 		}
// 		else if (command_type == "MODE")
// 		{
// 			std::string	arguments[3];

// 			line_ss >> arguments[0] >> arguments[1] >> arguments[2];

// 			if (arguments[2].size() || arguments[0].empty())
// 			{
// 				// error
// 			}
// 			else
// 			{
// 				if (arguments[0].at(0) == '#')
// 				{
// 					user	cur_user = Users.search_user_by_ident(cur_event.ident);
// 					Udata	tmp = Channels.mode_channel(cur_user, arguments[0], (arguments[1].size() == 1 && arguments[1] == "b"));
// 					push_write_event(tmp, cur_event);
// 				}
// 				// else
// 				// {
// 				// 	// Udata	tmp = Users.command_mode(arguments[0], (arguments[1].size () == 2 && arguments[1] == "+i"));
// 				// 	push_write_event(tmp, cur_event);
// 				// }
// 			}
// 		}
// 		else if (command_type == "WHO")
// 		{
// 			std::string	arguments[2];

// 			line_ss	>> arguments[0] >> arguments[1];
// 			if (arguments[0].empty() || arguments[1].size())
// 			{
// 				// error
// 			}
// 			else
// 			{
// 				// Udata	tmp = Channels.who_channel(arguments[0]);
// 				// push_write_event(tmp, cur_event);
// 			}
// 		}
// 		else if (command_type == "PART")
// 		{
// 			std::string chan_name, msg;

// 			line_ss >> chan_name >> msg;
// 			user leaver = Users.search_user_by_ident(cur_event.ident);
// 			std::vector<Udata>	udata_events = Channels.leave_channel(leaver, chan_name, msg);
// 			push_write_event_with_vector(udata_events);
// 		}
// 		else if (command_type == "TOPIC")
// 		{
// 			std::string chan_name, topic;
// 			line_ss >> chan_name >> topic;

// 			try
// 			{
// 				user sender = Users.search_user_by_ident(cur_event.ident);
// 				std::size_t	pos = line.find(':');
// 				topic = set_message_(line, pos + 1, (line.length() - (pos + 2)));
// 				(topic.size() > 510) ? topic.resize(510) : topic.resize(topic.size());

// 				std::vector<Udata>	udata_events = Channels.set_topic(sender, chan_name, topic);
// 				push_write_event_with_vector(udata_events);
// 			}
// 			catch (std::exception &e)
// 			{
// 				std::cout << "FUck" << std::endl;
// 			}
// 		}
// 		else if (command_type == "KICK")
// 		{
// 			std::string chan_name, target_name, msg;

// 			line_ss >> chan_name >> target_name;
// 			try
// 			{
// 				user kicker = Users.search_user_by_ident(cur_event.ident);
// 				user target = Users.search_user_by_nick(target_name);
// 				std::size_t	pos = line.find(':');
// 				msg = set_message_(line, pos + 1, (line.length() - (pos + 2)));
// 				(msg.size() > 510) ? msg.resize(510) : msg.resize(msg.size());
// 				std::vector<Udata>	udata_events = Channels.kick_channel(kicker, target, chan_name, msg);
// 				push_write_event_with_vector(udata_events);
// 			}
// 			catch (std::exception &e)
// 			{
// 				std::cout << "FUck" << std::endl;
// 			}
// 		}
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
				std::cout << BOLDGREEN
						  << "socket: " << udata_[i].sock_fd << "\nmsg: " << udata_[i].msg
						  << RESET << std::endl;
				send(cur_event.ident, udata_[i].msg.c_str(), udata_[i].msg.length(), 0);
				udata_.erase(udata_.begin() + i);
			}
		}
		return (1);
	}
	return (0);
}
