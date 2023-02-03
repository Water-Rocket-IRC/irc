// #pragma once

#ifndef PARSER_HPP
# define PARSER_HPP

#include <cstring>
#include <unistd.h>

#include "Udata.hpp"
#include "Users.hpp"
#include "Channels.hpp"

enum e_val { N_COMMAND = 13 };

class Parser
{
	private:
		Udata			&parser_udata_;
		Users			users_;
		Channels		channels_;

		static const std::string	commands[N_COMMAND];
		static void (Parser::*func_ptr[N_COMMAND])(const uintptr_t&, std::stringstream&, std::string&, const std::string&);

		std::string			set_message_(std::string &msg, size_t start, size_t end);
		std::string			message_resize_(std::stringstream& line_ss, std::string& to_send);

		void				push_write_event_(Event& tmp);
		void				push_multiple_write_events_(Udata& tmp, const uintptr_t& ident);

		void				parser_nick_(const uintptr_t& ident, std::stringstream& line_ss, std::string& to_send, const std::string& cmd);
		void				parser_user_(const uintptr_t& ident, std::stringstream& line_ss, std::string& to_send, const std::string& cmd);
		void				parser_ping_(const uintptr_t& ident, std::stringstream& line_ss, std::string& to_send, const std::string& cmd);
		void				parser_quit_(const uintptr_t& ident, std::stringstream& line_ss, std::string& to_send, const std::string& cmd);
		void				parser_privmsg_(const uintptr_t& ident, std::stringstream& line_ss, std::string& to_send, const std::string& cmd);
		void				parser_notice_(const uintptr_t& ident, std::stringstream& line_ss, std::string& to_send, const std::string& cmd);
		void				parser_wall_(const uintptr_t& ident, std::stringstream& line_ss, std::string& to_send, const std::string& cmd);
		void				parser_join_(const uintptr_t& ident, std::stringstream& line_ss, std::string& to_send, const std::string& cmd);
		void				parser_mode_(const uintptr_t& ident, std::stringstream& line_ss, std::string& to_send, const std::string& cmd);
		void				parser_who_(const uintptr_t& ident, std::stringstream& line_ss, std::string& to_send, const std::string& cmd);
		void				parser_part_(const uintptr_t& ident, std::stringstream& line_ss, std::string& to_send, const std::string& cmd);
		void				parser_topic_(const uintptr_t& ident, std::stringstream& line_ss, std::string& to_send, const std::string& cmd);
		void				parser_kick_(const uintptr_t& ident, std::stringstream& line_ss, std::string& to_send, const std::string& cmd);

		void				valid_user_checker_(const uintptr_t& ident, const std::string& cmd);
		const std::string	command_toupper(const char* command);

	public:
		Parser(Udata& serv_udata);
		~Parser();

		void			command_parser(const uintptr_t& ident, std::string &command);
};




#endif