#include "Parser.hpp"
#include "Receiver.hpp"
#include "Udata.hpp"
#include "color.hpp"

#include <sstream>
#include <string>
#include <sys/_types/_size_t.h>

const std::string Parser::commands[N_COMMAND] = {"PASS", "NICK", "USER", "PING", "JOIN", "QUIT", "PRIVMSG", "KICK", "PART", "TOPIC", "NOTICE"};
void (Parser::*Parser::func_ptr[N_COMMAND])(const uintptr_t&, std::stringstream&, std::string&) = \
								{&Parser::parser_pass_, &Parser::parser_nick_, &Parser::parser_user_, &Parser::parser_ping_, &Parser::parser_join_, &Parser::parser_quit_, &Parser::parser_privmsg_, \
								&Parser::parser_kick_, &Parser::parser_part_, &Parser::parser_topic_, &Parser::parser_notice_};

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
	database_.bot_maker("BOT");
}

Parser::~Parser()
{
}

// ctrl + c 눌렀을 때  지우는 것 
void	Parser::error_situation(const uintptr_t& ident)
{
	User&		tmp_usr = database_.select_user(ident);

	database_.delete_error_user(tmp_usr);
}

std::string	Parser::set_message_(std::string& msg, const std::size_t& start) // nl(10) is not included
{
	std::size_t	pos = msg.find('\r');
	pos = (pos == std::string::npos) ? msg.length() - start : pos - start;
	std::string	ret = msg.substr(start, pos);
	return ret;
}

std::string	Parser::message_resize_(const std::string& tmp, const std::string& to_send)
{
	std::string	ret;
	if (tmp.size() && tmp.at(0) == ':')
		ret = to_send;
	else
		ret = tmp;
	(ret.size() > 510) ? ret.resize(510) : ret.resize(ret.size());
	return ret;
}

void Parser::print_title(const std::string& title)
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

void	Parser::command_parser(const uintptr_t& ident, std::string& command)
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
		print_title(command_type);
		for (; i < N_COMMAND && (command_type != Parser::commands[i]); ++i) { }
		if (i < N_COMMAND)
		{
			std::size_t	pos(line.find(':'));
			std::string	to_send;
			if (pos == std::string::npos)
				to_send.clear();
			else
				to_send = set_message_(line, pos + 1);
			std::cout << " ///command_parser -> to_send : " << to_send << std::endl;
			(this->*Parser::func_ptr[i])(ident, line_ss, to_send);
		}
		else
		{
			std::cerr << BOLDRED << "We don't support these : \n" << RESET << command_type << std::endl;
			Event	tmp;
			tmp.first = ident;
			push_write_event_(tmp);
		}
	}
}

void	Parser::parser_pass_(const uintptr_t& ident, std::stringstream& line_ss, std::string& to_send)
{
	static_cast<void>(to_send);
	std::string	pw;
	Event		ret;

	ret.first = ident;
	getline(line_ss, pw);
	std::size_t	pos = pw.find('\r');
	pw = pw.substr(1, pos - 1);
	if (pw.empty())
	{
		ret = Sender::command_empty_argument_461(ident, "PASS");
	}
	else if (password_ != pw)
	{
		ret = Sender::password_incorrect_464(ident);
	}
	else
	{
		ret = database_.command_pass(ident);
	}
	push_write_event_(ret);
}

//check_argument(2, )
// NICK은 채널에 있을 때 모에게 NICK 변경되었음을 알리고, 로비에 있을 때는 본인에게만 알린다.
void	Parser::parser_nick_(const uintptr_t& ident, std::stringstream& line_ss, std::string& to_send)
{
	static_cast<void>(to_send);
	std::string	nick;
	Udata		ret;

	line_ss >> nick;
	if (nick.empty()) // no nickname error
	{
		Event	tmp = Sender::command_empty_argument_461(ident, "NICK");
		push_write_event_(tmp);
		return ;
	}
	ret = database_.command_nick(ident, nick);
	push_multiple_write_events_(ret, ident, 0);
}

void	Parser::parser_user_(const uintptr_t& ident, std::stringstream& line_ss, std::string& real_name)
{
	Udata		ret;
	Event		tmp;
	std::string	argument[3];
	std::string	real;

	line_ss >> argument[0] >> argument[1] >> argument[2] >> real;
	for (std::size_t i(0); i < 3; ++i)
	{
		if (argument[i].empty() || (argument[i].size() && argument[i].at(0) == ':'))
		{
			tmp =  Sender::command_empty_argument_461(ident, "USER");
			push_write_event_(tmp);
			return ;
		}
	}
	real = message_resize_(real, real_name);
	tmp = database_.command_user(ident, argument[0], argument[1], argument[2], real);
	ret.insert(tmp);
	push_multiple_write_events_(ret, ident, 0);
}

void	Parser::parser_ping_(const uintptr_t& ident, std::stringstream& line_ss, std::string& to_send)
{
	Event		ret;
	std::string msg, target;

	line_ss >> msg >> target;
	msg = message_resize_(msg, to_send);
	// std::cout << "what the hell are you doing? " <<  msg << std::endl;
	if (msg.empty())
	{
		ret =  Sender::command_empty_argument_461(ident, "PING");
		push_write_event_(ret);
		return ;
	}
	else if (msg.find(' ') == std::string::npos)
	{
		target.clear();
	}
	ret = database_.command_pong(ident, target, msg);
	push_write_event_(ret);
}

void	Parser::parser_quit_(const uintptr_t& ident, std::stringstream& line_ss, std::string& to_send)
{
	Udata		ret;
	std::string	nick, tmp;
	
	while (line_ss >> tmp)
	{
		if (nick.size() && nick.at(0) != ':')
		{
			nick = tmp + " ";
		}
	}
	nick += to_send;
	ret = database_.command_quit(ident, to_send);
	push_multiple_write_events_(ret, ident, 1);
}

// TODO : not yet
void	Parser::parser_privmsg_(const uintptr_t& ident, std::stringstream& line_ss, std::string& to_send)
{
	std::string	target, msg;
	Udata		ret;

	line_ss >> target;
	line_ss >> std::ws;
	std::getline(line_ss, msg);
	msg = message_resize_(msg, to_send);
	ret = database_.command_privmsg(ident, target, msg);
	push_multiple_write_events_(ret, ident, 0);
}

void	Parser::parser_notice_(const uintptr_t& ident, std::stringstream& line_ss, std::string& to_send)
{
	std::string	target, msg;
	Udata		ret;

	line_ss >> target;
	line_ss >> std::ws;
	std::getline(line_ss, msg);
	if (msg[0] != ':')
	{
		std::cout << "WHAT THE FUCK ARE YOU DOING?\n";
		// Event	tmp = Sender::wall_argument_error();
		// ret.insert(tmp);
	}
	else
	{
		msg = message_resize_(msg, to_send);
		ret = database_.command_notice(ident, target, msg);
	}
	push_multiple_write_events_(ret, ident, 0);
}

void	Parser::parser_join_(const uintptr_t& ident, std::stringstream& line_ss, std::string& to_send)
{
	static_cast<void>(to_send);
	std::string	chan_name;
	Udata		ret;

	line_ss >> chan_name;
	ret = database_.command_join(ident, chan_name);
	push_multiple_write_events_(ret, ident, 2);
}

// void	Parser::parser_mode_(const uintptr_t& ident, std::stringstream& line_ss, std::string& to_send)
// {
// 	static_cast<void>(to_send);
// 	valid_user_checker_(ident, cmd);
// 	std::string	target, mode;

// 	line_ss >> target >> mode;
// 	if (target.empty())
// 	{
// 		user&	tmp_user = users_.search_user_by_ident(ident, 461);	// <- '*' 461로 넣어야?
// 		throw Sender::command_empty_argument_461(tmp_user, "MODE");
// 	}
// 	user	cur_user = users_.search_user_by_ident(ident, 451);
// 	Udata	ret;
// 	if (target.at(0) == '#')
// 	{
// 		ret = channels_.mode_channel(cur_user, target, (mode.size() == 1 && mode == "b"));
// 	}
// 	else
// 	{
// 		ret = users_.command_mode(target, (mode.size () == 2 && mode == "+i"));
// 	}
// 	push_multiple_write_events_(ret, ident);
// }

// void	Parser::parser_who_(const uintptr_t& ident, std::stringstream& line_ss, std::string& to_send)
// {
// 	static_cast<void>(to_send);
// 	valid_user_checker_(ident, cmd);
// 	std::string	target;

// 	line_ss	>> target;
// 	if (target.empty())
// 	{
// 		user&	tmp_user = users_.search_user_by_ident(ident, 461);	// <- '*' 461로 넣어야?
// 		throw Sender::command_empty_argument_461(tmp_user, "MODE");
// 	}
// 	user&	asker = users_.search_user_by_ident(ident, 461);
// 	Udata	ret = channels_.who_channel(asker, target);
// 	push_multiple_write_events_(ret, ident);
// }

void	Parser::parser_part_(const uintptr_t& ident, std::stringstream& line_ss, std::string& to_send)
{
	Udata ret;
	std::string chan_name, msg;
	
	line_ss >> chan_name;
	line_ss >> std::ws;
	std::getline(line_ss, msg);
	msg = message_resize_(msg, to_send);
	User parter = database_.select_user(ident);
	ret = database_.part_channel(parter, chan_name, to_send);
	
	push_multiple_write_events_(ret, ident, 2);
}


void	Parser::parser_topic_(const uintptr_t& ident, std::stringstream& line_ss, std::string& to_send)
{
	Udata ret;
	std::string chan_name, msg;

	line_ss >> chan_name;
	line_ss >> std::ws;
	std::getline(line_ss, msg);
	msg = message_resize_(msg, to_send);
	User cur_usr = database_.select_user(ident);
	ret = database_.set_topic(cur_usr, chan_name, to_send);
	push_multiple_write_events_(ret, ident, 2);
}

void	Parser::parser_kick_(const uintptr_t& ident, std::stringstream& line_ss, std::string& to_send)
{
	Udata ret;
	Event tmp;
	std::string chan_name, target_name, msg;

	line_ss >> chan_name >> target_name >> msg;
	line_ss >> std::ws;
	msg = message_resize_(msg, to_send);	
	ret = database_.command_kick(ident, target_name, chan_name, to_send);
	push_multiple_write_events_(ret, ident, 2);
}

void	Parser::push_write_event_(Event& ret)
{
	if (ret.second.empty())
	{
		Receiver::get_Kevent_Handler().set_read(ret.first);
		return ;
	}
	parser_udata_.insert(ret);
	(Receiver::get_Kevent_Handler()).set_write(ret.first);
}

void	Parser::push_multiple_write_events_(Udata& ret, const uintptr_t& ident, const int flag)
{
	Udata_iter	target = ret.find(ident);

	if (ret.empty())
	{
		Receiver::get_Kevent_Handler().set_read(ident);
		return ;
	}
	if (target != ret.end())
	{
		if (target->second.empty() && flag == 0)
		{
			Receiver::get_Kevent_Handler().set_read(ident);
		}
		else if (flag == 1)
		{
			int	i(0);
			parser_udata_.insert(*target);
			Receiver::get_Kevent_Handler().set_exit(target->first, &i);
		}
		else
		{
			parser_udata_.insert(*target);
			Receiver::get_Kevent_Handler().set_write(target->first);
		}
	}
	// 내가 없으면 나를 제외한 모두에게 보냄
	for (Udata_iter iter = ret.begin(); iter != ret.end(); ++iter)
	{
		if (target != ret.end() && iter->first == ident)
		{
			continue ;
		}
		parser_udata_.insert(*iter);
		(Receiver::get_Kevent_Handler()).set_write(iter->first);
	}
}
