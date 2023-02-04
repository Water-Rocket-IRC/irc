#include "Parser.hpp"
#include "Receiver.hpp"
#include "Sender.hpp"
#include "Udata.hpp"
#include "user.hpp"
#include <netdb.h>
#include <sys/_types/_size_t.h>
#include <sys/_types/_uintptr_t.h>

// const std::string Parser::commands[N_COMMAND] = {"PASS", "NICK", "USER", "PING", "QUIT", "PRIVMSG", "NOTICE", "WALL", "JOIN", "MODE", "WHO", "PART", "TOPIC", "KICK"};
// void (Parser::*Parser::func_ptr[N_COMMAND])(const uintptr_t&, std::stringstream&, std::string&, const std::string&) = \
								{&Parser::parser_pass_, &Parser::parser_nick_, &Parser::parser_user_, &Parser::parser_ping_, &Parser::parser_quit_, &Parser::parser_privmsg_, &Parser::parser_notice_, \
								 &Parser::parser_wall_, &Parser::parser_join_, &Parser::parser_mode_, &Parser::parser_who_,  &Parser::parser_part_, &Parser::parser_topic_, &Parser::parser_kick_};

const std::string Parser::commands[N_COMMAND] = {"PASS", "NICK", "USER", "PING", "JOIN", "MODE", "WHO", "PART"};
void (Parser::*Parser::func_ptr[N_COMMAND])(const uintptr_t&, std::stringstream&, std::string&, const std::string&) = \
								{&Parser::parser_pass_, &Parser::parser_nick_, &Parser::parser_user_, &Parser::parser_ping_, \
							    &Parser::parser_join_, &Parser::parser_mode_, &Parser::parser_who_,  &Parser::parser_part_};

const std::string Parser::command_toupper(const char* command)
{
	std::string	ret;
	
	for (std::size_t i(0); i < std::strlen(command); ++i)
	{
		ret += std::toupper(static_cast<int>(command[i]));
	}
	return ret;
}

Parser::Parser(Udata& serv_udata, const std::string& password)
: parser_udata_(serv_udata), password_(password)
{
}

Parser::~Parser()
{
}

std::string	Parser::set_message_(std::string& msg, size_t start, size_t end)
{
	std::string	ret = msg.substr(start, end);
	return ret;
}

std::string	Parser::message_resize_(std::stringstream& line_ss, std::string& to_send)
{
	std::string tmp;
	line_ss >> tmp;
	std::string	ret = to_send.size() ? to_send : tmp;
	(ret.size() > 510) ? ret.resize(510) : ret.resize(ret.size());
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

void	Parser::command_parser(const uintptr_t& ident, std::string &command)
{
	std::stringstream	ss(command);
	std::string			line;

	while (std::getline(ss, line, '\n'))
	{
		std::stringstream	line_ss(line);
		std::string			command_type;
		std::size_t 		i(0);

		line_ss >> command_type;
		command_type = command_toupper(command_type.c_str());
		_print_title(command_type);
		for (; i < N_COMMAND; ++i)
		{
			if (command_type == Parser::commands[i])
			{
				std::size_t	pos(line.find(':'));
				std::string	to_send;
				if (pos == std::string::npos)
					to_send.clear();
				to_send = set_message_(line, pos + 1, (line.length() - (pos + 2)));
				try
				{
					(this->*Parser::func_ptr[i])(ident, line_ss, to_send, command_type);
				}
				catch (Event& e)
				{
					push_write_event_(e);
				}
				break ;
			}
		}
		if (i == N_COMMAND)
			std::cerr << BOLDRED << "We don't support these : \n" << RESET << command_type << std::endl;
	}
}

void	Parser::valid_user_checker_(const uintptr_t& ident, const std::string& command_type)
{
	try
	{
		user&	cur_user = users_.search_user_by_ident(ident, 0);
		if (!users_.has_nick(ident))
		{
			throw Sender::command_not_registered_451(ident, command_type);
		}
		else if (!users_.has_username(ident))
		{
			throw Sender::command_not_registered_451(cur_user, command_type);
		}
	}
	catch (std::exception& e) { }
}

void	Parser::parser_pass_(const uintptr_t& ident, std::stringstream& line_ss, std::string& to_send, const std::string& cmd)
{
	// TODO: have to make this
}


	//check_argument(2, )
	// NICK은 채널에 있을 때 모에게 NICK 변경되었음을 알리고, 로비에 있을 때는 본인에게만 알린다.
void	Parser::parser_nick_(const uintptr_t& ident, std::stringstream& line_ss, std::string& to_send, const std::string& cmd)
{
	static_cast<void>(to_send);
	std::string	nick;
	Udata		ret;

	line_ss >> nick;
	if (nick.empty()) // no nickname error
	{
		throw Sender::command_empty_argument_461(ident, "NICK");
	}
	if (nick.size() && nick.at(0) == '#')
	{
		user&	tmp_user = users_.search_user_by_ident(ident, 432);
		throw Sender::nick_wrong_message(tmp_user, nick);
	}
	ret = users_.command_nick(nick, ident);
	if (ret.empty())
	{
		// Client first enter
		Event	ret;

		ret.first = ident;
		push_write_event_(ret);
		return ;
	}
	// Success change nick
	user& 	who = users_.search_user_by_ident(ident, 451);
	Udata	ret_events = channels_.nick_channel(who, ret[ident]);
	ret.insert(ret_events.begin(), ret_events.end());
}

void	Parser::parser_user_(const uintptr_t& ident, std::stringstream& line_ss, std::string& real_name, const std::string& cmd)
{
	Event		ret;
	std::string	argument[4];
	
	line_ss >> argument[0] >> argument[1] >> argument[2] >> argument[3];
	for (size_t i(0); i < 4; ++i)
	{
		if (real_name.empty() || argument[i].empty())
		{
			throw Sender::command_empty_argument_461(ident, "USER");
		}
	}
	user&	cur_user = users_.search_user_by_ident(ident, 0); // NO THROW
	if (real_name.size())
	{
		argument[3] = real_name;
	}
	ret = users_.command_user(argument, ident); // USER TODO: have to change argument and logic
	push_write_event_(ret);
}

void	Parser::parser_ping_(const uintptr_t& ident, std::stringstream& line_ss, std::string& to_send, const std::string& cmd)
{
	valid_user_checker_(ident, cmd);
	static_cast<void>(to_send);
	std::string serv_addr;
	Event		ret;

	line_ss >> serv_addr;
	if (serv_addr.empty())
	{
		throw Sender::command_empty_argument_461(ident, "PING");
	}
	user&	cur_user = users_.search_user_by_ident(ident, 451); // NO SUCH USER
	if (serv_addr.size() == 1 && serv_addr.at(0) == ':' && to_send.empty())
	{
		throw Sender::command_no_origin_specified_409(cur_user, "PING");
	}
	ret = Sender::pong(ident, serv_addr);
	push_write_event_(ret);
}

void	Parser::parser_quit_(const uintptr_t& ident, std::stringstream& line_ss, std::string& to_send, const std::string& cmd)
{
	valid_user_checker_(ident, cmd);
	Udata		ret;
	const std::string	msg = message_resize_(line_ss, to_send);

	user&	cur_user = users_.search_user_by_ident(ident, 1); // QUIT 할 때 유저가 없음
	ret = channels_.quit_channel(cur_user, msg); // USER TODO: Have to change logic
	if (channels_.is_user_in_channel(cur_user))
	{
		ret = channels_.quit_channel(cur_user, msg);
	}
	else
	{
		ret = users_.command_quit(cur_user, msg);
	}
	push_multiple_write_events_(ret, ident);
}

// TODO : not yet
void	Parser::parser_privmsg_(const uintptr_t& ident, std::stringstream& line_ss, std::string& to_send, const std::string& cmd)
{
	valid_user_checker_(ident, cmd);
	std::string target;

	line_ss >> target;
	const std::string msg = message_resize_(line_ss, to_send);

	(to_send.size() > 510) ? to_send.resize(510) : to_send.resize(to_send.size());
	if (target.at(0) == '#')
	{
		try
		{
			user&	sender = users_.search_user_by_ident(ident); // USER is unregistered
			
			try
			{
				Channels&	ret_chan = Channels::select_channel(user); // TODO: user is not in channel
				// if success
				std::vector<Udata>	udata_events = channels_.channel_msg(sender, target, msg);
				push_multiple_write_events_(udata_events);
			}
			catch(const std::exception& e)
			{
				Udata	ret = Sender::privmsg_no_user_error_message(sender, target);
				push_write_event_(ret, ident);
			}
			
		}
		catch(const std::exception& e)
		{
			Udata	ret = Sender::privmsg_no_user_error_message_in_channel(); // TODO: no user in channel
			push_write_event_(ret, ident);
		}
	}
	else
	{
		Udata	ret = users_.command_privmsg(target, to_send, ident); // TODO: USER have to change logic
		push_write_event_(ret, ident);
	}
}

// TODO : not yet
void	Parser::parser_notice_(const uintptr_t& ident, std::stringstream& line_ss, std::string& to_send, const std::string& cmd)
{
	valid_user_checker_(ident, cmd);
	std::string target;

	line_ss >> target;
	const std::string msg = message_resize_(line_ss, to_send);

	if (target.at(0) == '#')
	{
		try
		{
			user&	sender = users_.search_user_by_ident(ident); // USER is unregistered
			
			try
			{
				Channels&	ret_chan = Channels::select_channel(sender); // TODO: user is not in channel
				// if success
				std::vector<Udata>	udata_events = channels_.channel_notice(sender, target, msg);
				push_multiple_write_events_(udata_events);
			}
			catch(const std::exception& e)
			{
				Udata	ret = Sender::notice_no_user_error_message(sender, target); // TODO: no user found with notice
				push_write_event_(ret, ident);
			}
			
		}
		catch(const std::exception& e)
		{
			Udata	ret = Sender::notice_no_user_error_message_in_channel(); // TODO: no user in channel
			push_write_event_(ret, ident);
		}
	}
	else
	{
		Udata	ret = users_.command_notice(target, to_send, ident); // TODO: USER have to change logic
		push_write_event_(ret, ident);
	}
}
// TODO : not yet
void	Parser::parser_wall_(const uintptr_t& ident, std::stringstream& line_ss, std::string& to_send, const std::string& cmd)
{
	valid_user_checker_(ident, cmd);
	std::string chan_name;

	line_ss >> chan_name;
	const std::string msg = message_resize_(line_ss, to_send);
	if (msg[0] != ':')
	{
		//오류
	}
	else
	{
		try
		{
			user sender = users_.search_user_by_ident(ident);

			std::size_t	pos = line.find(':');
			msg = set_message_(line, pos + 1, (line.length() - (pos + 2)));
			(msg.size() > 510) ? msg.resize(510) : msg.resize(msg.size());

			std::vector<Udata>	udata_events = channels_.channel_notice(sender, chan_name, msg);
			push_multiple_write_events_(udata_events);
		}
		catch (std::exception &e)
		{
			std::cout << "FUck" << std::endl;
		}
	}
}

void	Parser::parser_join_(const uintptr_t& ident, std::stringstream& line_ss, std::string& to_send, const std::string& cmd)
{
	static_cast<void>(to_send);
	valid_user_checker_(ident, cmd);
	std::string	chan_name;
	Udata		ret;

	line_ss >> chan_name;
	if (chan_name.empty())
	{
		user&	tmp_user = users_.search_user_by_ident(ident, 461);	// <- '*' 461로 넣어야?
		throw Sender::command_empty_argument_461(tmp_user, "MODE");
	}
	user&	cur_user = users_.search_user_by_ident(ident, 451); // JOIN 시 해당 닉네임의 유저가 없음
	if (chan_name.at(0) != '#')
		throw Sender::join_invaild_channel_name_message(user, chan_name)
	ret = channels_.join_channel(cur_user, chan_name);
	push_multiple_write_events_(ret, ident);
}

void	Parser::parser_mode_(const uintptr_t& ident, std::stringstream& line_ss, std::string& to_send, const std::string& cmd)
{
	static_cast<void>(to_send);
	valid_user_checker_(ident, cmd);
	std::string	target, mode;

	line_ss >> target >> mode;
	if (target.empty())
	{
		user&	tmp_user = users_.search_user_by_ident(ident, 461);	// <- '*' 461로 넣어야?
		throw Sender::command_empty_argument_461(tmp_user, "MODE");
	}
	user	cur_user = users_.search_user_by_ident(ident, 451);
	Udata	ret;
	if (target.at(0) == '#')
	{
		ret = channels_.mode_channel(cur_user, target, (mode.size() == 1 && mode == "b"));
	}
	else
	{
		ret = users_.command_mode(target, (mode.size () == 2 && mode == "+i"));
	}
	push_multiple_write_events_(ret, ident);
}

void	Parser::parser_who_(const uintptr_t& ident, std::stringstream& line_ss, std::string& to_send, const std::string& cmd)
{
	static_cast<void>(to_send);
	valid_user_checker_(ident, cmd);
	std::string	target;

	line_ss	>> target;
	if (target.empty())
	{
		user&	tmp_user = users_.search_user_by_ident(ident, 461);	// <- '*' 461로 넣어야?
		throw Sender::command_empty_argument_461(tmp_user, "MODE");
	}
	user&	asker = users_.search_user_by_ident(ident, 461);
	Udata	ret = channels_.who_channel(asker, target);
	push_multiple_write_events_(ret, ident);
}

void	Parser::parser_part_(const uintptr_t& ident, std::stringstream& line_ss, std::string& to_send, const std::string& cmd)
{
	valid_user_checker_(ident, cmd);
	std::string chan_name;


	line_ss >> chan_name;
	const std::string msg = message_resize_(line_ss, to_send);

	user& leaver = users_.search_user_by_ident(ident);
	std::vector<Udata>	udata_events = channels_.leave_channel(leaver, chan_name, msg);
	push_multiple_write_events_(udata_events);
}

void	Parser::parser_topic_(const uintptr_t& ident, std::stringstream& line_ss, std::string& to_send, const std::string& cmd)
{
	valid_user_checker_(ident, cmd);
	std::string chan_name;
	line_ss >> chan_name;

	const std::string topic = message_resize_(line_ss, to_send);
	try
	{
		user sender = users_.search_user_by_ident(ident);
		std::size_t	pos = line.find(':');
		topic = set_message_(line, pos + 1, (line.length() - (pos + 2)));
		(topic.size() > 510) ? topic.resize(510) : topic.resize(topic.size());

		Udata	udata_events = channels_.set_topic(sender, chan_name, topic);
		// push_multiple_write_events_(udata_events); -> todo 바꿔야함
	}
	catch (std::exception &e)
	{
		std::cout << "FUck" << std::endl;
	}
}

void	Parser::parser_kick_(const uintptr_t& ident, std::stringstream& line_ss, std::string& to_send, const std::string& cmd)
{
	valid_user_checker_(ident, cmd);
	std::string chan_name, target_name;

	line_ss >> chan_name >> target_name;
	const std::string msg = message_resize_(line_ss, to_send);
	try
	{
		user kicker = users_.search_user_by_ident(ident);
		user target = users_.search_user_by_nick(target_name);
		std::size_t	pos = line.find(':');
		msg = set_message_(line, pos + 1, (line.length() - (pos + 2)));
		(msg.size() > 510) ? msg.resize(510) : msg.resize(msg.size());
		std::vector<Udata>	udata_events = channels_.kick_channel(kicker, target, chan_name, msg);
		push_multiple_write_events_(udata_events);
	}
	catch (std::exception &e)
	{
		std::cout << "FUck" << std::endl;
	}
}

void	Parser::push_write_event_(Event& ret)
{
	if (ret.second.size())
	{
		parser_udata_.insert(ret);
		(Receiver::get_Kevent_Handler()).set_write(ret.first);
	}
}

void	Parser::push_multiple_write_events_(Udata& ret, const uintptr_t& ident)
{
	Udata_iter	target = ret.find(ident); //이게 없으면 내 자신한테 보내는게 아님

	// 내가 있으면 나를 먼저 보내고 모두에게 보냄
	if (target != ret.end())
	{
		Receiver::get_Kevent_Handler().set_write(ident);
	}
	// 내가 없으면 나를 제외한 모두에게 보냄
	for (Udata_iter iter = ret.begin(); iter != ret.end(); ++iter)
	{
		parser_udata_.insert(*iter);
		if (target != ret.end() && iter->first == ident)
		{
			continue ;
		}
		(Receiver::get_Kevent_Handler()).set_write(iter->first);
	}
}

// 혹시 멀티플 write에서 빈 Udata를 해야되나?
