#pragma once

#include <cstring>
#include <unistd.h>

#include "Database.hpp"

class Receiver;

enum e_val { N_COMMAND = 11 };

class Parser
{
	private:
		Udata&				parser_udata_;
		Database			database_;
		const std::string&	password_;

		static const std::string	commands[N_COMMAND];
		static void (Parser::*func_ptr[N_COMMAND])(const uintptr_t&, std::stringstream&, std::string&);

		std::string			set_message_(std::string &msg, const std::size_t& start);
		std::string			message_resize_(const std::string& tmp, const std::string& to_send);
		void				push_write_event_(Event& tmp);
		void				push_multiple_write_events_(Udata& tmp, const uintptr_t& ident, const int flag);
		void				parser_pass_(const uintptr_t& ident, std::stringstream& line_ss, std::string& to_send);
		void				parser_nick_(const uintptr_t& ident, std::stringstream& line_ss, std::string& to_send);
		void				parser_user_(const uintptr_t& ident, std::stringstream& line_ss, std::string& to_send);
		void				parser_ping_(const uintptr_t& ident, std::stringstream& line_ss, std::string& to_send);
		void				parser_quit_(const uintptr_t& ident, std::stringstream& line_ss, std::string& to_send);
		void				parser_privmsg_(const uintptr_t& ident, std::stringstream& line_ss, std::string& to_send);
		void				parser_notice_(const uintptr_t& ident, std::stringstream& line_ss, std::string& to_send);
		void				parser_wall_(const uintptr_t& ident, std::stringstream& line_ss, std::string& to_send);
		void				parser_join_(const uintptr_t& ident, std::stringstream& line_ss, std::string& to_send);
		void				parser_mode_(const uintptr_t& ident, std::stringstream& line_ss, std::string& to_send);
		void				parser_who_(const uintptr_t& ident, std::stringstream& line_ss, std::string& to_send);
		void				parser_part_(const uintptr_t& ident, std::stringstream& line_ss, std::string& to_send);
		void				parser_topic_(const uintptr_t& ident, std::stringstream& line_ss, std::string& to_send);
		void				parser_kick_(const uintptr_t& ident, std::stringstream& line_ss, std::string& to_send);
		const std::string	command_toupper(const char* command);

	public:
		Parser(Udata& serv_udata, const std::string& password);

		void			clear_all();
		void			command_parser(const uintptr_t& ident, std::string& command);
		void			error_situation(const uintptr_t& ident);
};