// #pragma once

#ifndef PARSER_HPP
# define PARSER_HPP

#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/event.h>
#include <sys/types.h>
#include <netdb.h>

class Receiver;

enum e_val { N_COMMAND = 13 };

class Parser
{
	private:
		static const std::string	commands[N_COMMAND];
		static void (Parser::*func_ptr[N_COMMAND])(struct kevent&, std::stringstream&, std::string&);
		std::string	set_message_(std::string &msg, size_t start, size_t end);
	public:
		Parser(/* args */);
		~Parser();
		void	command_parser(struct kevent &cur_event, std::string &command);

		void	parser_nick(struct kevent& cur_event, std::stringstream& line_ss, std::string& to_send);
		void	parser_user(struct kevent& cur_event, std::stringstream& line_ss, std::string& to_send);
		void	parser_ping(struct kevent& cur_event, std::stringstream& line_ss, std::string& to_send);
		// void	parser_quit(struct kevent& cur_event, std::stringstream& line_ss, std::string& to_send);
		// void	parser_privmsg(struct kevent& cur_event, std::stringstream& line_ss, std::string& to_send);
		// void	parser_notice(struct kevent& cur_event, std::stringstream& line_ss, std::string& to_send);
		// void	parser_wall(struct kevent& cur_event, std::stringstream& line_ss, std::string& to_send);
		void	parser_join(struct kevent& cur_event, std::stringstream& line_ss, std::string& to_send);
		void	parser_mode(struct kevent& cur_event, std::stringstream& line_ss, std::string& to_send);
		void	parser_who(struct kevent& cur_event, std::stringstream& line_ss, std::string& to_send);
		void	parser_part(struct kevent& cur_event, std::stringstream& line_ss, std::string& to_send);
		// void	parser_topic(struct kevent& cur_event, std::stringstream& line_ss, std::string& to_send);
		// void	parser_kick(struct kevent& cur_event, std::stringstream& line_ss, std::string& to_send);
};




#endif