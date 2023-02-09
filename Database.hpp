#pragma once

#include "Channel.hpp"
#include "Sender.hpp"
#include "Udata.hpp"

/** 데이터베이스 클래스는 채널들과 유저들의 정보를 관리하고 요청에 대한 적절한 응답을 생성한다 **/
class Database
{
	private:
		std::vector<Channel> 	channel_list_;
		std::vector<User>		user_list_;

	public:
		std::vector<User>&		get_user_list(void);
		Event					valid_user_checker_(const uintptr_t& ident, const std::string& command_type);
	
	/** Channel **/
		Udata					channel_msg(User& sender, std::string chan_name, const std::string& msg);
		Udata					notice_channel(User& sender, std::string chan_name, const std::string& msg);
		Udata					wall_channel(User& sender, std::string chan_name, const std::string& msg);
		bool					is_channel(std::string& chan_name);
		bool					is_user_in_channel(User& leaver);
		Channel& 				create_channel(User& joiner, std::string& chan_name, std::string chan_access);
		void 					delete_channel(std::string& chan_name);
		Channel&				select_channel(std::string& chan_name);
		Channel&				select_channel(User& connector);
		Udata					set_topic(User& sender, std::string& chan_name, std::string& topic);
		Udata					kick_channel(User& host, User& target, std::string& chan_name, std::string& msg);
		Udata				 	join_channel(User& joiner, const std::string& chan_name_);
		Udata					part_channel(User&leaver, std::string& chan_name, const std::string& msg_);
		Udata					quit_channel(User&leaver, std::string& chan_name, const std::string& msg_);
		Udata					nick_channel(User& who, std::string& new_nick);
		std::vector<Channel>&	get_channels() { return	channel_list_; };
	/** User **/
		void					delete_error_user(const uintptr_t& ident);
		Event					command_pass(const uintptr_t& ident);
		Udata					command_nick(const uintptr_t& ident, std::string& nick_name);
		Event					command_user(const uintptr_t& ident, const std::string& username, const std::string& mode,
								const std::string& unused, const std::string& realname);
		Event					command_pong(const uintptr_t& ident, const std::string& target, const std::string& msg);
		Udata					command_quit(const uintptr_t& ident, const std::string& chan_name);
		Udata					command_privmsg(const uintptr_t& ident, const std::string &target_name, const std::string &msg);
		Udata					command_notice(const uintptr_t& ident, const std::string &target_name, const std::string &msg);
		Event					command_privmsg(std::string &target_name, std::string &line, const uintptr_t& ident);
		Udata					command_join(const uintptr_t& ident, const std::string& chan_name);
		Udata					command_part(const uintptr_t& ident, std::string& chan_name, const std::string& msg);
		Udata					command_kick(const uintptr_t &ident, const std::string& target_name, std::string& chan_name, std::string& msg);
		bool					is_user(const uintptr_t& ident);
		bool					is_user(const std::string& nickname);
		User&					select_user(const uintptr_t& ident);
		User&					select_user(const std::string& nickname);
		void					delete_user(User& leaver);
		bool 					is_duplicate_ident(const uintptr_t& ident);
		bool 					is_duplicate_nick(std::string& nick_name);
		bool					does_has_nickname(const uintptr_t& ident);
		bool					does_has_username(const uintptr_t& ident);
		bool					is_valid_nick(std::string& new_nick);
	/** bot **/
		Event					bot_privmsg(User&	cur_usr, const std::string &msg);
		void					bot_maker(const std::string& name);
};
