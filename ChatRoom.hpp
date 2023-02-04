#pragma once

#include "Sender.hpp"
#include "Udata.hpp"
#include <vector>
#include <algorithm>
#include <sstream>
#include "ChatUser.hpp"

enum e_send_switch { JOIN, PART, PRIV, KICK, QUIT, NOTICE, TOPIC, WALL, NICK, MODE, WHO};
/*
 * JOIN : 나를 포함해서 모두에게 보여주는 메세지(메세지가 다름)
 * PART : 나를 포함해서 모두에게 보여주는 메세지(메세지가 다름)
 * PRIV : 채팅방에서 그냥 작성하는 메세지 (나한테 보여지는 건 client가 해주는 것)
 */

class ChatRoom
{
	private:
		std::string				name_;
		std::string				topic_;
		int						error_;
		ChatUser				host_;				// operator
		std::vector<ChatUser>	connectors_;		// connectors.at(0) => operator
		std::string				access_;


//TODO: 메시지를 전송하는 모든 명령은 udata로 리턴할 것. Udata.hpp 참고
	public:
		std::string				get_info(void);
		std::string&			get_name(void);
		ChatUser&				get_host(void);
		void					set_host(void);
		std::string&			get_access(void);
		void					set_access(const std::string& access);
		void					set_topic(std::string& topic);
		bool 					is_user(ChatUser& usr);
		void					change_nick(ChatUser& usr, std::string new_nick);
		void 					add_user(ChatUser& joiner);
		void 					set_channel_name(std::string& chan_name);
		void					delete_user(ChatUser& usr);
		std::string				get_user_list_str(void);


		Udata					send_all(ChatUser& sender, ChatUser& target, std::string msg, int remocon);

		std::vector<ChatUser>&	get_users(void);
		std::vector<ChatUser>	sort_users(void);
		void					seterror();
		int						geterror();
		bool operator==(const ChatRoom& t) const;
};
