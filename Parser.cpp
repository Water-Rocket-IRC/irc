#include "Parser.hpp"
#include "Receiver.hpp"

// const std::string Parser::commands[N_COMMAND] = {"NICK", "USER", "PING", "QUIT", "PRIVMSG", "NOTICE", "WALL", "JOIN", "MODE", "WHO", "PART", "TOPIC", "KICK"};
// void (Parser::*Parser::func_ptr[N_COMMAND])(struct kevent&, std::stringstream&, std::string&) = \
// 								{&Parser::parser_nick, &Parser::parser_user, &Parser::parser_ping, &Parser::parser_quit, &Parser::parser_privmsg, &Parser::parser_notice
// 							   , &Parser::parser_wall, &Parser::parser_join, &Parser::parser_mode, &Parser::parser_who,  &Parser::parser_part, &Parser::parser_topic, &Parser::parser_kick };
const std::string Parser::commands[N_COMMAND] = {"NICK", "USER", "PING", "JOIN", "MODE", "WHO", "PART"};
void (Parser::*Parser::func_ptr[N_COMMAND])(struct kevent&, std::stringstream&, std::string&) = \
								{&Parser::parser_nick, &Parser::parser_user, &Parser::parser_ping
							   , &Parser::parser_join, &Parser::parser_mode, &Parser::parser_who,  &Parser::parser_part };
	// ret = channel.send_all(moder, trash, chan_name, MODE);

Parser::Parser(/* args */)
{
}

Parser::~Parser()
{
}

std::string	Parser::set_message_(std::string &msg, size_t start, size_t end)
{
	std::string	ret = msg.substr(start, end);
	return ret;
}

static void _print_title(const std::string& title)
{

	std::string  under_bar = "____";
	std::string  over_bar =  "‾‾‾‾";
	for (std::size_t i = 0; i < title.length(); ++i)
	{
		under_bar += "_";
		over_bar += "‾";
	}
	std::cout << BOLDCYAN << under_bar
			  << std::endl << "| " << title << " |"
			  << std::endl << over_bar
			  << RESET << std::endl;
}

void	Parser::command_parser(struct kevent &cur_event, std::string &command)
{
	std::stringstream	ss(command);
	std::string			line;

	while (std::getline(ss, line, '\n'))
	{
		std::stringstream	line_ss(line);
		std::string			command_type;

		line_ss >> command_type;
		_print_title(command_type);
		for (int i(0); i < N_COMMAND; ++i)
		{
			if (command_type == Parser::commands[i])
			{
				std::size_t	pos(line.find(':'));
				std::string	to_send;
				if (pos == std::string::npos)
					to_send.clear();
				to_send = set_message_(line, pos + 1, (line.length() - (pos + 2)));
				(this->*Parser::func_ptr[i])(cur_event, line_ss, to_send);
			}
			else 
			{
				std::cerr << BOLDRED << "We don't support these : \n" << RESET << command_type << std::endl;
			}
		}
	}
}


	//check_argument(2, )
	// NICK은 채널에 있을 때 모에게 NICK 변경되었음을 알리고, 로비에 있을 때는 본인에게만 알린다.
void	Parser::parser_nick(struct kevent &cur_event, std::stringstream& line_ss, std::string& to_send)
{
	static_cast<void>(to_send);
	std::string	nick;
	Udata		tmp;

	bzero(&tmp, sizeof(tmp));
	line_ss >> nick;
	if (nick.empty()) // no nickname error
	{
		tmp = Sender::command_empty_argument_461(cur_event.ident, "NICK"); // TODO: name respecify
		Receiver::push_write_event(tmp, cur_event);
		return ;
	}
	tmp = Users_.command_nick(nick, cur_event.ident);
	if (tmp.msg.empty()) {
		// Client first enter
		push_write_event(tmp, cur_event);
		return ;
	}
	std::vector<Udata>	events;
	events.push_back(tmp);
	try
	{
		// Success change nick
		user& 	who = Users_.search_user_by_ident(cur_event.ident);
		std::vector<Udata> tmp_events = Channels.nick_channel(who, tmp.msg);
		events.insert(events.end(), tmp_events.begin(), tmp_events.end());
		push_write_event_with_vector(events);
	}
	catch (const std::exception& e)
	{
		// cannot found who want to change the nick
		push_write_event(tmp, cur_event);
	}
}

void	Parser::parser_user(struct kevent &cur_event, std::stringstream& line_ss, std::string& real_name)
{

	Udata	tmp;
	std::string	argument[4];
	
	bzero(&tmp, sizeof(tmp));
	line_ss >> argument[0] >> argument[1] >> argument[2] >> argument[3];
	for (size_t i(0); i < 4; ++i)
	{
		if (argument[i].empty())
		{
			try
			{
				user&	cur_user = Users_.search_user_by_ident(cur_event.ident);
				if (cur_user.is_user_has_nick())
				{
					tmp = Sender::command_not_registered_451(cur_user, "USER");
					push_write_event(tmp, cur_event);
					return ;
				}
				tmp = Sender::command_empty_argument_461(cur_user, "USER"); // TODO: low argument with user command
				push_write_event(tmp, cur_event);
			}
			catch(const std::exception&)
			{
				tmp = Sender::command_not_registered_451(cur_event.ident, "USER");
				push_write_event(tmp, cur_event);
			}
		}
	}
	if (real_name.size())
	{
		argument[3] = real_name;
	}
	tmp = Users_.command_user(argument, cur_event.ident); // USER TODO: have to change argument and logic
	push_write_event(tmp, cur_event);
}

void	Parser::parser_ping(struct kevent &cur_event, std::stringstream& line_ss, std::string& to_send)
{
	std::string serv_addr;
	Udata		tmp;

	line_ss >> serv_addr;
	bzero(&tmp, sizeof(tmp));
	// if (serv_addr.empty())
	// {
	// 	tmp = Sender::ping_461_error(); // TODO: ping no parameter
	// 	push_write_event(tmp, cur_event);
	// 	return ;
	// }
	// if (serv_addr.size() == 1 && serv_addr.at(0) == ':' && to_send.empty())
	// {
	// 	tmp = Sender::ping_409_error(); // TODO: ping empty parameter
	// 	push_write_event(tmp, cur_event);
	// 	return ;
	// }
	try
	{
		user&	cur_user = Users_.search_user_by_ident(cur_event.ident);

		tmp = Sender::pong(cur_event.ident, serv_addr);
		push_write_event(tmp, cur_event);
	}
	catch(const std::exception& e)
	{
		// tmp = Sender::pong_451_error(); // TODO: ping no user found
		// push_write_event(tmp, cur_event);
	}
}

// void	Parser::parser_quit(struct kevent &cur_event, std::stringstream& line_ss, std::string& to_send)
// {
// 	std::string	nick, msg;
// 	Udata		tmp;

// 	line_ss >> nick >> msg;
// 	bzero(&tmp, sizeof(tmp));
// 	try
// 	{
// 		user&	cur_user = Users_.search_user_by_ident(cur_event.ident);

// 		tmp = Users_.command_quit(nick, to_send, cur_event.ident); // USER TODO: Have to change logic
// 		push_write_event(tmp, cur_event);
// 	}
// 	catch(const std::exception& e)
// 	{
// 		tmp = Sender::user_quit_error(); // TODO: no user found
// 		push_write_event(tmp, cur_event);
// 	}
	
// }

// void	Parser::parser_privmsg(struct kevent &cur_event, std::stringstream& line_ss, std::string& to_send)
// {
// 	std::string target, msg;

// 	line_ss >> target >> msg;
// 	(to_send.size() > 510) ? to_send.resize(510) : to_send.resize(to_send.size());
// 	if (target.at(0) == '#')
// 	{
// 		try
// 		{
// 			user&	sender = Users_.search_user_by_ident(cur_event.ident); // USER is unregistered
			
// 			try
// 			{
// 				Channels&	tmp_chan = Channels::select_channel(user); // TODO: user is not in channel
// 				// if success
// 				std::vector<Udata>	udata_events = Channels.channel_msg(sender, target, msg);
// 				push_write_event_with_vector(udata_events);
// 			}
// 			catch(const std::exception& e)
// 			{
// 				Udata	tmp = Sender::privmsg_no_user_error_message(sender, target);
// 				push_write_event(tmp, cur_event);
// 			}
			
// 		}
// 		catch(const std::exception& e)
// 		{
// 			Udata	tmp = Sender::privmsg_no_user_error_message_in_channel(); // TODO: no user in channel
// 			push_write_event(tmp, cur_event);
// 		}
// 	}
// 	else
// 	{
// 		Udata	tmp = Users_.command_privmsg(target, to_send, cur_event.ident); // TODO: USER have to change logic
// 		push_write_event(tmp, cur_event);
// 	}
// }

// void	Parser::parser_notice(struct kevent &cur_event, std::stringstream& line_ss, std::string& to_send)
// {
// 	std::string target, msg;

// 	line_ss >> target >> msg;
// 	(to_send.size() > 510) ? to_send.resize(510) : to_send.resize(to_send.size());
// 	if (target.at(0) == '#')
// 	{
// 		try
// 		{
// 			user&	sender = Users_.search_user_by_ident(cur_event.ident); // USER is unregistered
			
// 			try
// 			{
// 				Channels&	tmp_chan = Channels::select_channel(sender); // TODO: user is not in channel
// 				// if success
// 				std::vector<Udata>	udata_events = Channels.channel_notice(sender, target, msg);
// 				push_write_event_with_vector(udata_events);
// 			}
// 			catch(const std::exception& e)
// 			{
// 				Udata	tmp = Sender::notice_no_user_error_message(sender, target); // TODO: no user found with notice
// 				push_write_event(tmp, cur_event);
// 			}
			
// 		}
// 		catch(const std::exception& e)
// 		{
// 			Udata	tmp = Sender::notice_no_user_error_message_in_channel(); // TODO: no user in channel
// 			push_write_event(tmp, cur_event);
// 		}
// 	}
// 	else
// 	{
// 		Udata	tmp = Users_.command_notice(target, to_send, cur_event.ident); // TODO: USER have to change logic
// 		push_write_event(tmp, cur_event);
// 	}
// }

// void	Parser::parser_wall(struct kevent &cur_event, std::stringstream& line_ss, std::string& to_send)
// {
// 	std::string chan_name, msg;

// 	line_ss >> chan_name >> msg;
// 	if (msg[0] != ':')
// 	{
// 		//오류
// 	}
// 	else
// 	{
// 		try
// 		{
// 			user sender = Users_.search_user_by_ident(cur_event.ident);

// 			std::size_t	pos = line.find(':');
// 			msg = set_message_(line, pos + 1, (line.length() - (pos + 2)));
// 			(msg.size() > 510) ? msg.resize(510) : msg.resize(msg.size());

// 			std::vector<Udata>	udata_events = Channels.channel_notice(sender, chan_name, msg);
// 			push_write_event_with_vector(udata_events);
// 		}
// 		catch (std::exception &e)
// 		{
// 			std::cout << "FUck" << std::endl;
// 		}
// 	}
// }

void	Parser::parser_join(struct kevent &cur_event, std::stringstream& line_ss, std::string& to_send)
{
	std::string	chan_name, error;


	line_ss >> chan_name >> error;

	if (!error.empty())
	{
		//error 나중에 nc로 넣어보기
	}
	else
	{
		try
		{
			user chan_user = Users_.search_user_by_ident(cur_event.ident);
			std::vector<Udata>	udata_events = Channels.join_channel(chan_user, chan_name);
			push_write_event_with_vector(udata_events);
		}
		catch(const std::exception& e)
		{
			// Udata	tmp = Sender::no_user_found_with_join(); // TODO: 
		}
		
	}

}

void	Parser::parser_mode(struct kevent &cur_event, std::stringstream& line_ss, std::string& to_send)
{
	std::string	arguments[3];

	line_ss >> arguments[0] >> arguments[1] >> arguments[2];

	if (arguments[2].size() || arguments[0].empty())
	{
		// error
	}
	else
	{
		if (arguments[0].at(0) == '#')
		{
			user	cur_user = Users_.search_user_by_ident(cur_event.ident);
			Udata	tmp = Channels.mode_channel(cur_user, arguments[0], (arguments[1].size() == 1 && arguments[1] == "b"));
			push_write_event(tmp, cur_event);
		}
		else
		{
			// Udata	tmp = Users_.command_mode(arguments[0], (arguments[1].size () == 2 && arguments[1] == "+i"));
			// push_write_event(tmp, cur_event);
		}
	}
}

void	Parser::parser_who(struct kevent &cur_event, std::stringstream& line_ss, std::string& to_send)
{
	std::string	arguments[2];

	line_ss	>> arguments[0] >> arguments[1];
	if (arguments[0].empty() || arguments[1].size())
	{
		// error
	}
	else
	{
		Udata	tmp = Channels.who_channel(cur_event.ident, arguments[0]);
		push_write_event(tmp, cur_event);
	}
}

void	Parser::parser_part(struct kevent &cur_event, std::stringstream& line_ss, std::string& to_send)
{
	std::string chan_name, msg;

	line_ss >> chan_name >> msg;
	user leaver = Users_.search_user_by_ident(cur_event.ident);
	std::vector<Udata>	udata_events = Channels.leave_channel(leaver, chan_name, msg);
	push_write_event_with_vector(udata_events);
}

// void	Parser::parser_topic(struct kevent &cur_event, std::stringstream& line_ss, std::string& to_send)
// {
// 	std::string chan_name, topic;
// 	line_ss >> chan_name >> topic;

// 	try
// 	{
// 		user sender = Users_.search_user_by_ident(cur_event.ident);
// 		std::size_t	pos = line.find(':');
// 		topic = set_message_(line, pos + 1, (line.length() - (pos + 2)));
// 		(topic.size() > 510) ? topic.resize(510) : topic.resize(topic.size());

// 		std::vector<Udata>	udata_events = Channels.set_topic(sender, chan_name, topic);
// 		push_write_event_with_vector(udata_events);
// 	}
// 	catch (std::exception &e)
// 	{
// 		std::cout << "FUck" << std::endl;
// 	}
// }

// void	Parser::parser_kick(struct kevent &cur_event, std::stringstream& line_ss, std::string& to_send)
// {
// 	std::string chan_name, target_name, msg;

// 	line_ss >> chan_name >> target_name;
// 	try
// 	{
// 		user kicker = Users_.search_user_by_ident(cur_event.ident);
// 		user target = Users_.search_user_by_nick(target_name);
// 		std::size_t	pos = line.find(':');
// 		msg = set_message_(line, pos + 1, (line.length() - (pos + 2)));
// 		(msg.size() > 510) ? msg.resize(510) : msg.resize(msg.size());
// 		std::vector<Udata>	udata_events = Channels.kick_channel(kicker, target, chan_name, msg);
// 		push_write_event_with_vector(udata_events);
// 	}
// 	catch (std::exception &e)
// 	{
// 		std::cout << "FUck" << std::endl;
// 	}
// }
