// #pragma once

#ifndef PARSER_HPP
# define PARSER_HPP

#include <iostream>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/event.h>
#include <sys/types.h>
#include <netdb.h>

#include "Users.hpp"
#include "Channels.hpp"

enum e_val { N_COMMAND = 13 };

class Parser
{
	private:
		Users			users_;
		Channels		channels_;

		static const std::string	commands[N_COMMAND];
		static void (Parser::*func_ptr[N_COMMAND])(uintptr_t&, std::stringstream&, std::string&);

		std::string	set_message_(std::string &msg, size_t start, size_t end);

		void		push_write_event_(Udata &tmp, struct kevent &cur_event);
		void		push_multiple_write_events_(std::vector<Udata>& udata_events);

		void		parser_nick_(uintptr_t& ident, std::stringstream& line_ss, std::string& to_send);
		void		parser_user_(uintptr_t& ident, std::stringstream& line_ss, std::string& to_send);
		void		parser_ping_(uintptr_t& ident, std::stringstream& line_ss, std::string& to_send);
		// void		parser_quit_(uintptr_t& ident, std::stringstream& line_ss, std::string& to_send);
		// void		parser_privmsg_(uintptr_t& ident, std::stringstream& line_ss, std::string& to_send);
		// void		parser_notice_(uintptr_t& ident, std::stringstream& line_ss, std::string& to_send);
		// void		parser_wall_(uintptr_t& ident, std::stringstream& line_ss, std::string& to_send);
		void		parser_join_(uintptr_t& ident, std::stringstream& line_ss, std::string& to_send);
		void		parser_mode_(uintptr_t& ident, std::stringstream& line_ss, std::string& to_send);
		void		parser_who_(uintptr_t& ident, std::stringstream& line_ss, std::string& to_send);
		void		parser_part_(uintptr_t& ident, std::stringstream& line_ss, std::string& to_send);
		// void		parser_topic_(uintptr_t& ident, std::stringstream& line_ss, std::string& to_send);
		// void		parser_kick_(uintptr_t& ident, std::stringstream& line_ss, std::string& to_send);

	public:
		Parser(/* args */);
		~Parser();

		void	command_parser(uintptr_t& ident, std::string &command);
};




#endif