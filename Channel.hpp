#pragma once

#include "Sender.hpp"
#include "Udata.hpp"

#include <vector>
#include <algorithm>
#include <sstream>

enum e_send_switch { JOIN, PART, PRIV, KICK, QUIT, NOTICE, TOPIC, WALL, NICK, MODE, WHO};

/** 채널 클래스는 각 채널의 정보 관리 및 수정을 담당한다. **/
class Channel
{
	private:
		User				host_;
		std::vector<User>	connectors_;
		std::string			name_;
		std::string			topic_;
		std::string			access_;
		int					error_;

	public:
		std::string&		get_name(void);
		std::string&		get_topic(void);
		User&				get_host(void);
		void				set_host(void);
		void				set_host(User& new_host);
		std::string&		get_access(void);
		void				set_access(const std::string& access);
		void				set_topic(std::string& topic);
		bool 				is_user(User& usr);
		void				change_nick(User& usr, std::string new_nick);
		void 				add_user(User& joiner);
		void 				set_channel_name(std::string& chan_name);
		void				delete_user(User& usr);
		std::string			get_user_list_str(void);

		Udata				send_all(User& sender, User& target, std::string msg, int remocon);

		std::vector<User>&	get_users(void);
		std::vector<User>	sort_users(void);
		void				seterror();
		int					geterror();
		bool operator==(const Channel& t) const;
};
