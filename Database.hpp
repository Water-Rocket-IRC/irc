#pragma once

#include "Channel.hpp"

/*********************** TODO: 왜 채널의 기본 생성자가 없다고 뜨죠...?? *********************/
class Database
{
	private:
		std::vector<Channel> Channels_;
		std::vector<User>	user_list_;

	public:
	
	// Channel
		Udata					channel_msg(User& sender, std::string chan_name, std::string& msg);
		Udata					channel_notice(User& sender, std::string chan_name, std::string& msg);
		Udata					channel_wall(User& sender, std::string chan_name, std::string& msg);

		bool					is_channel(std::string& chan_name);
		bool					is_user_in_channel(User& leaver);
		void 					create_channel(User& joiner, std::string& chan_name, std::string chan_access);
		void 					delete_channel(std::string& chan_name);
		Channel&				select_channel(std::string& chan_name, int error_code, User& sender, std::string& command);
		Channel&				select_channel(User& connector, int error_code, std::string& command);

		Udata					set_topic(User& sender, std::string& chan_name, std::string& topic);
		Udata					kick_channel(User& host, User& target, std::string& chan_name, std::string& msg);
		Udata					quit_channel(User& target, std::string msg);
		Udata				 	join_channel(User& joiner, std::string& chan_name);
		Udata					leave_channel(User&leaver, std::string& chan_name, std::string& msg);
		Udata					nick_channel(User& who, std::string& send_msg);
		Udata					mode_channel(User& moder, const std::string& chan_name, const bool vaild);
		Udata					who_channel(User& asker, std::string& chan_name);//(const uintptr_t& sock, std::string& chan_name);
		std::vector<Channel>&	get_channels() { return	Channels_; };

	//User
		Event	command_nick(std::string& nick_name, const uintptr_t& ident);
		Event	command_user(const std::string input[4], const uintptr_t& ident);
		Event	command_quit(User& leaver, const std::string& leave_msg);
		Event	command_privmsg(std::string &target_name, std::string &line, const uintptr_t& ident);
		Event	command_mode(std::string &target_name, int flag);

		User&	search_user_by_ident(const uintptr_t& ident, int error_code);
		User&	search_user_by_nick(std::string nickname, int error_code);
		void	delete_user(User& leaver);
		bool 	is_duplicate_ident(const uintptr_t& ident);
		bool 	is_duplicate_nick(std::string& nick_name);

		bool	has_nick(const uintptr_t& ident);
		bool	has_username(const uintptr_t& ident);

		void	print_all_user(); //debug
};
