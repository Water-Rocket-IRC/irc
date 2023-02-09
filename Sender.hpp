#pragma once

#include "Udata.hpp"
#include "User.hpp"
#include "color.hpp"

class Sender
{ 
	public:
		static Event		unknown_command_message_421(const User& sender, const std::string& command);
		static Event		pong(const uintptr_t& socket, const std::string& target, const std::string& msg);
		static Event		command_no_origin_specified_409(const User& sender, const std::string& command);
		static Event		command_not_registered_451(const User& sender, const std::string& command);
		static Event		command_not_registered_451(const uintptr_t& sock, const std::string& command);
		static Event		command_empty_argument_461(const User& sender, const std::string& command);
		static Event		command_empty_argument_461(const uintptr_t& sock, const std::string& command);
		static Event		welcome_message_connect(const User& sender);
		static Event		nick_well_message(const User& sender, const User& receiver, const std::string& new_nick);
		static Event		nick_error_message(const User& sender, const std::string& new_nick);
		static Event		nick_error_message2(const User& sender, const std::string& new_nick);
		static Event		nick_error_message(const uintptr_t& sock, const std::string& new_nick);
		static Event		nick_wrong_message(const User& sender, const std::string& new_nick);
		static Event		nick_wrong_message(const uintptr_t& sock, const std::string& new_nick);
		static Event		password_incorrect_464(const uintptr_t& sock);
		static Event		password_not_yet_464(const uintptr_t& sock);
		static Event		quit_leaver_message(const User& sender, std::string leave_message);
		static Event		quit_member_message(const User& sender, const User& receiver, std::string leave_message);
		static Event		privmsg_bot_message(const User& sender, const std::string& msg);
		static Event		privmsg_p2p_message(const User& sender, const User& target, const std::string& msg);
		static Event		privmsg_channel_message(const User& sender, const User& receiver, const std::string& channel, const std::string& msg);
		static Event		privmsg_no_user_error_message(const User& sender, const std::string& target);
		static Event		privmsg_external_error_message(const User& sender, const std::string channel);
		static Event		join_message(const User& sender, const User& receiver, const std::string& channel);
		static Event		join_invaild_channel_name_message(const User& sender, const std::string invaild_channel);
		static Event		part_message(const User& sender, const User& receiver, const std::string& channel, const std::string& msg);
		static Event		kick_message(const User& sender, const User& receiver, const std::string& subject, const std::string& channel, const std::string& msg);
		static Event		kick_error_not_op_message(const User& sender, const std::string& host, const std::string& channel);
		static Event		kick_error_no_user_message(const User& sender, const std::string& host, const std::string& target, const std::string& channel);
		static Event		topic_message(const User& sender, const User& receiver, const std::string& channel, const std::string& topic);
		static Event		topic_error_message(const User& sender, const std::string& channel);
		static Event		notice_p2p_message(const User& sender, const User& target, const std::string& msg);
		static Event		notice_channel_message(const User& sender, const User& receiver, const std::string& channel, const std::string& msg);
		static Event		notice_no_nick_message(const User& sender, const User& receiver);
		static Event		wall_message(const User& sender, const User& receiver, const std::string& channel, const std::string& msg);
		static Event		no_channel_message(const User& sender, const std::string& channel);
		static Event		no_user_message(const User& sender, const std::string& target);
		static Event		mode_324_message(const User& sender, const std::string channel);
		static Event		mode_no_option_error_message(const User& sender, const std::string wrong_option);
		static Event		mode_not_operator_error_message(const User& sender, const std::string channel);
		static Event		who_joiner_352_message(const User& sender, const std::string channel);
		static std::string	who_352_target_message(const User& sender, const std::string channel, const std::string target);
		static std::string	who_315_message(const User& sender, const std::string channel);
		static std::string	mode_make_operator_message(const User& sender, std::string channel, const User& target);
		static std::string	mode_329_message(const User& sender, const std::string channel, const std::string time_stamp);
		static std::string	join_353_message(const User& sender, const std::string& chan_name, const std::string& chan_status, const std::string& chan_user_list);
		static std::string	join_366_message(const User& sender, const std::string& chan_name);
	private:
		static const std::string	server_name_;
};
