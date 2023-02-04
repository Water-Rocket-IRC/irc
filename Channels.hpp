#pragma once

#include "ChatRoom.hpp"
#include "Udata.hpp"
#include "Users.hpp"

/*********************** TODO: 왜 채널의 기본 생성자가 없다고 뜨죠...?? *********************/
class Channels
{
	private:
		std::vector<ChatRoom> Channels_;

	public:
	//TODO: 메시지를 전송하는 모든 명령은 udata로 리턴할 것. map안에 집어넣으면 됨. Udata hpp참고
		Channels(Users &users);
		Udata					channel_msg(ChatUser& sender, std::string chan_name, std::string& msg);
		Udata					channel_notice(ChatUser& sender, std::string chan_name, std::string& msg);
		Udata					channel_wall(ChatUser& sender, std::string chan_name, std::string& msg);

		bool					is_channel(std::string& chan_name);
		bool					is_user_in_channel(ChatUser& leaver);
		void 					create_channel(ChatUser& joiner, std::string& chan_name, std::string chan_access);
		void 					delete_channel(std::string& chan_name);
		ChatRoom&				select_channel(std::string& chan_name, int error_code, ChatUser& sender, std::string& command);
		ChatRoom&				select_channel(ChatUser& connector, int error_code, std::string& command);

		Udata					set_topic(ChatUser& sender, std::string& chan_name, std::string& topic);
		Udata					kick_channel(ChatUser& host, ChatUser& target, std::string& chan_name, std::string& msg);
		Udata					quit_channel(ChatUser& target, std::string msg);
		Udata				 	join_channel(ChatUser& joiner, std::string& chan_name);
		Udata					leave_channel(ChatUser&leaver, std::string& chan_name, std::string& msg);
		Udata					nick_channel(ChatUser& who, std::string& send_msg);
		Udata					mode_channel(ChatUser& moder, const std::string& chan_name, const bool vaild);
		Udata					who_channel(ChatUser& asker, std::string& chan_name);//(const uintptr_t& sock, std::string& chan_name);
		std::vector<ChatRoom>&	get_channels() { return	Channels_; };
};
