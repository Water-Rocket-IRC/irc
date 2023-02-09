#include "Sender.hpp"

const std::string	Sender::server_name_ = "irc.local";
/****************************       <PING && PONG && USE && etc>       ****************************/

/** @brief PING명령에 대한 응답으로 PONG 전송s **/
Event	Sender::pong(const uintptr_t& socket, const std::string& target, const std::string& msg)
{
	Event ret;

	const std::string& cur_target = target.empty() ? target : Sender::server_name_;
	const std::string& pong_reply = ":" + Sender::server_name_ + " PONG " \
		+ cur_target + " :" + msg;
	ret = std::make_pair(socket, pong_reply + "\r\n");
	return ret;
}
/** @brief target을 operator로 만들 때 보내는 패킷 매시지 **/
std::string	Sender::mode_make_operator_message(const User& sender, std::string channel, const User& target)
{
	Event		ret;

	std::string mode_msg = ":" + sender.nickname_ + "!" + sender.mode_ \
	+ " MODE " + channel + " +o :" + target.nickname_;
	return (mode_msg + "\r\n");
}

/** @brief 461 - 충분한 수의 parameter가 오지 않았을 때 보내는 패킷 매세지 **/
Event	Sender::command_empty_argument_461(const User& sender, const std::string& command)
{
	Event ret;

	const std::string& error_message = ":" + Sender::server_name_ + " 461 " \
		+ sender.nickname_ + " " + command  + " :Not enough parameters";
	ret = std::make_pair(sender.client_sock_, error_message + "\r\n");
	return ret;
}

/** @brief 461 - 충분한 수의 parameter가 오지 않았을 때 보내는 패킷 메세지 **/
Event	Sender::command_empty_argument_461(const uintptr_t& sock, const std::string& command)
{
	Event ret;

	const std::string& error_message = ":" + Sender::server_name_ + " 461 * " \
	+ command  + " :Not enough parameters";
	ret = std::make_pair(sock, error_message + "\r\n");
	return ret;
}

/** @brief 451 - 클라이언트가 등록되지 않았을 때 보내는 오류 패킷 메세지 **/
Event	Sender::command_not_registered_451(const User& sender, const std::string& command)
{
	Event ret;

	const std::string& error_message = ":" + Sender::server_name_ + " 451 " \
		+ sender.nickname_ + " " + command  + " :You have not registered";
	ret = std::make_pair(sender.client_sock_, error_message + "\r\n");
	return ret;
}

/** @brief 451 - 클라이언트가 등록되지 않았을 때 보내는 오류 패킷 매세지 **/
Event	Sender::command_not_registered_451(const uintptr_t& sock, const std::string& command)
{
	Event ret;

	const std::string& error_message = ":" + Sender::server_name_ + " 451 * " \
		+ " " + command  + " :You have not registered";
	ret = std::make_pair(sock, error_message + "\r\n");
	return ret;
}

/** @brief 409 - PING or PONG message missing the originator parameter **/
Event	Sender::command_no_origin_specified_409(const User& sender, const std::string& command)
{
	Event ret;

	const std::string& error_message = ":" + Sender::server_name_ + " 409 " \
		+ sender.nickname_ + " " + command  + " :No origin specified";
	ret = std::make_pair(sender.client_sock_, error_message + "\r\n");
	return ret;
}

/** @brief 421 -  서버에 등록되지 않은 명령어를 입력할 경우 보내는 오류 패킷 메세지 **/
Event	Sender::unknown_command_message_421(const User& sender, const std::string& command)
{
	Event ret;

	const std::string& error_message = ":" + Sender::server_name_ + " 421 " \
		+ sender.nickname_ + " " + command  + " :Unknown command";
	ret = std::make_pair(sender.client_sock_, error_message + "\r\n");
	return ret;
}
// /****************************       <NICK>       ****************************/

/** @brief nickname이 정상적으로 바뀌었을 때 보내는 패킷 메시지 **/
Event	Sender::nick_well_message(const User& sender, const User& receiver, const std::string& new_nick) // 1st done
{
	Event	ret;

	const std::string& nick_msg = ":" + sender.nickname_ + "!" + sender.username_ \
					+ "@" + sender.unused_ + " NICK :" + new_nick;
	ret = std::make_pair(receiver.client_sock_, nick_msg + "\r\n");
	return ret;
}

/** @brief 904 - 잘못된 패드워드를 입력하였을 때 보내는 오류 패킷 메세지 **/
Event		Sender::password_incorrect_464(const uintptr_t& sock)
{
	Event	ret;

	const std::string& nick_msg = ":" + Sender::server_name_ + " 464 " + "User :Password Incorrect, Server disconnected"; \
	ret = std::make_pair(sock, nick_msg + "\r\n");
	return ret;	
}

/** @brief 433 - 이미 있는 nickname을 사용 혹은 그것으로 교체하려 할 때 보내는 오류 패킷 메세지 **/
Event	Sender::nick_error_message(const User& sender, const std::string& new_nick) // 1st done
{
	Event	ret;

	const std::string& nick_msg = ":" + Sender::server_name_ + " 433 " + sender.nickname_ \
					+ " " + new_nick + " :Nickname is already in use.";
	ret = std::make_pair(sender.client_sock_, nick_msg + "\r\n");
	return ret;
}

/** @brief 433 - nickname을 만드는 규칙에 위배될 때 보내는 오류 패킷 메세지 **/
Event	Sender::nick_error_message2(const User& sender, const std::string& new_nick) // 1st done
{
	Event	ret;

	const std::string& nick_msg = ":" + Sender::server_name_ + " 433 " + sender.nickname_ \
					+ " " + new_nick + " :Nickname overruled.";
	ret = std::make_pair(sender.client_sock_, nick_msg + "\r\n");
	return ret;
}

/** @brief 433 - 이미 있는 nickname을 사용 혹은 그것으로 교체하려 할 때 보내는 오류 패킷 메세지 **/
Event	Sender::nick_error_message(const uintptr_t& sock, const std::string& new_nick)
{
	Event	ret;

	const std::string& nick_msg = ":" + Sender::server_name_ + " 433 * " \
	+ new_nick + " :Nickname is already in use.";
	ret = std::make_pair(sock, nick_msg + "\r\n");
	return ret;
}

/** @brief 432 - nickname을 만드는 규칙에 위배될 때 보내는 오류 패킷 메세지 **/
Event	Sender::nick_wrong_message(const User& sender, const std::string& new_nick)
{
	Event	ret;

	const std::string& nick_msg = ":" + Sender::server_name_ + " 432 " + sender.nickname_ \
					+ " " + new_nick + " Erroneous Nickname.";
	ret = std::make_pair(sender.client_sock_, nick_msg + "\r\n");
	return ret;
}

/** @brief 432 - nickname을 만드는 규칙에 위배될 때 보내는 오류 패킷 메세지 **/
Event	Sender::nick_wrong_message(const uintptr_t& sock, const std::string& new_nick)
{
	Event	ret;

	const std::string& nick_msg = ":" + Sender::server_name_ + " 432 " + " * " \
					+ " " + new_nick + " Erroneous Nickname.";
	ret = std::make_pair(sock, nick_msg + "\r\n");
	return ret;
}
// /****************************       <Connect server || channel>       ****************************/

/** @brief connect 시 보내는 환영 패킷 메세지 **/
Event	Sender::welcome_message_connect(const User& sender)
{
	Event ret; 

	const std::string& msg001 = ":" + Sender::server_name_ + " 001 " + sender.nickname_ \
	+ " :Welcome to the 42's irc network " + sender.nickname_ + "!" + sender.mode_;
	ret = std::make_pair(sender.client_sock_, msg001 + "\r\n");
	return ret;
}

// /****************************       <QUIT>       ****************************/


/** @brief quit을 한 클라이언트에 보내는 패킷 메세지 **/
Event	Sender::quit_leaver_message(const User& sender, std::string leave_message)
{
	Event	ret;

	if (leave_message.empty())
	{
		leave_message = "leaving";
	}

	const std::string&  quit_channel_message = "ERROR :Closing link: (" \
		+ sender.nickname_ + ") [Quit: " + leave_message + "]";
	ret = std::make_pair(sender.client_sock_, quit_channel_message + "\r\n");
	return ret;
}

/** @brief quit을 한 클라이언트이외 같은 채널 내 다른 클라이언트들 보내는 패킷 메세지 **/
Event	Sender::quit_member_message(const User& sender, const User& receiver, std::string leave_message)
{
	Event	ret;

	if (leave_message.empty())
	{
		leave_message = "leaving";
	}
	const std::string&  quit_message = ":" + sender.nickname_ + "!" \
				+ sender.username_ + "@" + sender.unused_ + " QUIT :Quit: " + leave_message;
	ret = std::make_pair(receiver.client_sock_, quit_message + "\r\n");
	return ret;
}

// /****************************       <JOIN>       ****************************/

/**  @brief join 성공 시 보내는 패킷 메세지 **/
Event	Sender::join_message(const User& sender, const User& receiver, const std::string& channel)
{
	Event	ret;

	const std::string&  join_message = ":" + sender.nickname_ + "!" + sender.username_ + "@127.0.0.1 JOIN :" + channel;
	ret = make_pair(receiver.client_sock_, join_message + "\r\n");
	return ret;
}

/** @brief 476 - join 할 때 invaild한 채널 이름을 만들 경우 보내는 오류 패킷 메세지 ex) "#" 과 같은 이름 **/
Event	Sender::join_invaild_channel_name_message(const User& sender, const std::string invaild_channel)
{
	Event ret;

	const std::string&	error_message = ":" + Sender::server_name_ + " 476 " \
		+ sender.nickname_ + " " + invaild_channel  + " :Invaild channel name";
	ret = std::make_pair(sender.client_sock_, error_message + "\r\n");
	return ret;
}


/** @brief 353 - join 시 같이 보내는 패킷 메세지 **/
std::string	Sender::join_353_message(const User& sender, const std::string& chan_name
									, const std::string& chan_access, const std::string& chan_user_list)
{
	const std::string& 	ret = ":" + Sender::server_name_ + " 353 "+ sender.nickname_
							+ " " + chan_access + " " + chan_name + " :" + chan_user_list;
	return ret + "\r\n";
}



/** @brief 366 - join 시 같이 보내는 패킷 메세지 **/
std::string	Sender::join_366_message(const User& sender, const std::string& chan_name)
{
	const std::string& 	ret = ":" + Sender::server_name_ + " 366 "+ sender.nickname_
									+ " " + chan_name + " :End of /NAMES list.";
	return ret + "\r\n";
}

/** @brief 324 - join 할 때 mode 명령어 관련 패킷 메세지 **/
Event	Sender::mode_324_message(const User& sender, const std::string channel)
{
	Event	ret;

	const std::string&  mode_message = ":" + Sender::server_name_ \
			+ " 324 " + sender.nickname_ + " " + channel + ":+nt";
	ret = std::make_pair(sender.client_sock_, mode_message + "\r\n");
	return ret;
}
/** @brief 329 - join 할 때 mode 명령어 관련 패킷 메세지 **/
std::string	Sender::mode_329_message(const User& sender, const std::string channel, const std::string time_stamp)
{
	const std::string& ret = ":" + Sender::server_name_ + " 329 " \
			+ sender.nickname_ + " " + channel + " :" + time_stamp + "";
	return ret;
}
/** @brief 472 - mode 명령어 시 존재하지 않는 옵션을 넣을 때 보내는 오류 패킷 메세지 **/
Event	Sender::mode_no_option_error_message(const User& sender, const std::string channel)
{
	Event ret;

	const std::string& mode_message = ":" + Sender::server_name_ + " 472 " + sender.nickname_ \
		+ " " +channel + " :You must have channel op access or above to set channel mode p";
	ret = std::make_pair(sender.client_sock_, mode_message + "\r\n");
	return ret;
}
/** @brief 482 - operator가 아닌 클라이언트가 operator 권한이 필요한 명령어를 사용하려 할 때 보내는 오류 패킷 메세지 **/
Event	Sender::mode_not_operator_error_message(const User& sender, const std::string wrong_option)
{
	Event ret;

	const std::string& mode_message = ":" + Sender::server_name_ + " 482 " + sender.nickname_ \
		+ " " + wrong_option + " :is not a recognised channel mode";
	ret = std::make_pair(sender.client_sock_, mode_message + "\r\n");
	return ret;
}
 
 /** @brief 352 join 할때 joiner(조인 하는 자신)에게도 보내는 패킷 메세자 **/
Event	Sender::who_joiner_352_message(const User& sender, const std::string channel)
{
	Event ret;

	const std::string& who_message = ":" + Sender::server_name_ + " 352 " + sender.nickname_ \
			+ " " + channel + " " + sender.username_ + " " + sender.mode_ + " " \
			+ sender.unused_ + " " + sender.nickname_ + " H :0 root";
	ret = std::make_pair(sender.client_sock_, who_message + "\r\n");
	return ret;
}

/** @brief 352 who_joiner_352_message 함수 호출 직후에 같이 보내지는 패킷 메세지 **/
std::string	Sender::who_352_target_message(const User& sender, const std::string channel, const std::string target)
{
	const std::string& ret = ":" + Sender::server_name_ + " 352 " + sender.nickname_ \
			+ " " + channel + " " + sender.username_ + " " + sender.mode_ + " " \
			+ sender.unused_ + " " + target + " H :0 root";
	return ret;
}


/** @brief 315 who_352_target_message 함수 호출 직후에 같이 보내지는 패킷 메세지 **/
std::string	Sender::who_315_message(const User& sender, const std::string channel)
{
	const std::string& ret = ":" + Sender::server_name_ + " 315 " + sender.nickname_ \
		+ " " + channel + " :End of /WHO list";
	return ret;
}

/****************************       <PART>       ****************************/

/** @brief part 할 때 leaver 외에 다른 클라이언트들에게도 보내는 패킷 메세지 **/
Event	Sender::part_message(const User& sender, const User& receiver
							, const std::string& channel, const std::string& msg)
{
	Event	ret;

	const std::string&  part_message = ":" + sender.nickname_ + "!" \
				+ sender.username_ + "@" + receiver.mode_ + " PART " + channel + " " + msg + "";
	ret = std::make_pair(receiver.client_sock_, part_message + "\r\n");
	return ret;
}

/****************************       <KICK>       ****************************/

/** @brief kick 할 때 target외에 다른 클라이언트들에게 보내지는 패킷 메세지 **/
Event	Sender::kick_message(const User& sender, const User& receiver, const std::string& target
								, const std::string& channel, const std::string& msg)
{
	Event	ret;

	const std::string&  kick_message = ":" + sender.nickname_ + "!" + \
				sender.username_ + '@' + sender.unused_ + " KICK " + channel + " " + target + " :" + msg + "";
	ret = std::make_pair(receiver.client_sock_, kick_message + "\r\n");
	return ret;
}

/** @brief 482 - operator 권한이 없는 클라이언트가 kick 명령어를 사용하려 할 때 보내는 오류 메세지 **/
Event	Sender::kick_error_not_op_message(const User& sender, const std::string& host, const std::string& channel)
{
	Event	ret;

	 const std::string&  kick_message = ":" + Sender::server_name_ + \
	 	" 482 " + host + " " + channel + " :You must be a channel operator";
	ret = std::make_pair(sender.client_sock_, kick_message + "\r\n");
	return ret;
}

/** @brief 441 - kick을 당할 target 클라이언트가 존재하지 않을 경우 보내는 오류 패킷 메세지 **/
Event	Sender::kick_error_no_user_message(const User& sender, const std::string& host, const std::string& target, const std::string& channel)
{
	Event	ret;

	const std::string&  kick_message = ":" + Sender::server_name_ + \
		" 441 " + host + " " + target + " " + channel + " :They are not on that channel";
	ret = std::make_pair(sender.client_sock_, kick_message + "\r\n");
	return ret;
}

/****************************       <Privmsg>       ****************************/

/** @brief BOT을 call 했을 때 bot에게 보내는 패킷 메세지 **/
Event	Sender::privmsg_bot_message(const User& sender, const std::string& msg)
{
	Event		ret;

	const std::string& privmsg = ":BOT!dummy@" + sender.unused_ + " PRIVMSG " + \
		sender.nickname_ + " :" + msg;
	ret = std::make_pair(sender.client_sock_, privmsg + "\r\n");
	return (ret);
}

/** @brief target에게 private message를 보낼 때 target 클라이언트에게 보내는 패킷 메세지 **/
Event	Sender::privmsg_p2p_message(const User& sender, const User& target, const std::string& msg)
{
	Event		ret;

	const std::string& privmsg = ":" + sender.nickname_ + "!" + sender.username_ + "@" + sender.unused_ + " PRIVMSG " + \
		target.nickname_ + " :" + msg;
	ret = std::make_pair(target.client_sock_, privmsg + "\r\n");
	return (ret);
}

/** @brief target channel에 있는 receiver 클라이언트에게 private message를 보낼 때 클라이언트들에게 가는 패킷 메세지 **/ 
Event	Sender::privmsg_channel_message(const User& sender, const User& receiver, const std::string& msg, const std::string& channel)
{
	Event		ret;

	const std::string& privmsg = ":" + sender.nickname_ + "!" + sender.username_ + "@" + sender.unused_ + " PRIVMSG " + \
		channel + " :" + msg;
	ret = std::make_pair(receiver.client_sock_, privmsg + "\r\n");
	return (ret);
}

/** @brief 401 - privmsg를 존재하지 않는 target에게 보낼 때 클라이언트에게 보내지는 오류 패킷 메세지 **/
Event	Sender::privmsg_no_user_error_message(const User& sender, const std::string& target)
{
	Event		ret;

	const std::string& privmsg = ":" + sender.nickname_ + " 401 " + sender.nickname_ + " " + target \
	+ " :No such nick";
	ret = std::make_pair(sender.client_sock_, privmsg + "\r\n");
	return (ret);
}
 
/** @brief 404 - 채널 외부에 있는 클라이언트가 nc로 특정 채널에 메세지를 보낼 때 클라이언트에게 보내지는 오류 패킷 메세지 **/ 
Event	Sender::privmsg_external_error_message(const User& sender, const std::string channel)
{
	Event		ret;

	const std::string& privmsg = ":" + sender.nickname_ + " 404 " + sender.nickname_ + " " + channel \
	+ " :You cannot send external messages to this channel whilist the +n (noextmsg) mode is set";
	ret = std::make_pair(sender.client_sock_, privmsg + "\r\n");
	return (ret);
}

/****************************       <NOTICE && WALL>       ****************************/

/** @brief target 클라이언트에게 notice 명령어를 사용할 때 target 클라이언트에게 보내지는 패킷 메세지 **/
Event	Sender::notice_p2p_message(const User& sender, const User& target, const std::string& msg)
{
	Event		ret;

	const std::string& privmsg = ":" + sender.nickname_ + "@" + sender.unused_ + " NOTICE " + \
		target.nickname_ + " :" + msg;
	ret = std::make_pair(target.client_sock_, privmsg + "\r\n");
	return (ret);
}

/** @brief target channel에게 notice 를 할 때 receiver 클라이언트에게 보내지는 패킷 메세지 **/
Event	Sender::notice_channel_message(const User& sender, const User& receiver, const std::string& msg, const std::string& channel)
{
	Event		ret;

	const std::string& privmsg = ":" + sender.nickname_ + "@" + sender.unused_ + " NOTICE " + \
		channel + " :" + msg;
	ret = std::make_pair(receiver.client_sock_, privmsg + "\r\n");
	return (ret);
}

/** @brief  notice의 대상이 되는 receiver 클라이언트가 존재하지 않을 때 sender에게 보내는 패킷 메시지 **/
Event 	Sender::notice_no_nick_message(const User& sender, const User& receiver)
{
	Event		ret;

	const std::string& privmsg = ":" + Sender::server_name_ + " 401 " + sender.nickname_ + " " + receiver.username_ + \
	" No such nick";
	ret = std::make_pair(sender.client_sock_, privmsg + "\r\n");
	return (ret);
}

/** @brief  wall 할 때 메세지를 받는 receiver 클라이언트들에게 보내지는 패킷 메세지 **/
Event	Sender::wall_message(const User& sender, const User& receiver, const std::string& channel, const std::string& msg)
{
	Event		ret;

	const std::string& privmsg = ":" + sender.nickname_ + "!" + sender.username_ + "@" + receiver.unused_ + \
	" NOTICE @" + channel + " :" + msg;
	ret = std::make_pair(receiver.client_sock_, privmsg + "\r\n");
	return (ret);
}

/****************************       <TOPIC>       ****************************/

/** @brief topic이 변경됬음을 알리는 패킷 메시지 **/
Event	Sender::topic_message(const User& sender, const User& receiver, const std::string& channel, const std::string& topic)
{
	Event		ret;

	const std::string& topic_msg = ":" + sender.unused_ + "!" + sender.username_ + " TOPIC " + \
		channel + " " + topic;
	ret = std::make_pair(receiver.client_sock_, topic_msg + "\r\n");
	return (ret);
}

/** @brief 482 operator 권한이 없는 클라이언트가 topic 명령어를 사용할 때 보내자는 패킷 메세지 **/
Event	Sender::topic_error_message(const User& sender, const std::string& channel)
{
	Event		ret;

	const std::string& topic_msg = ":" + Sender::server_name_ + " 482 " + sender.nickname_ + " " + \
		channel + " " + "You do not have access to change the topic on this channel";
	ret = std::make_pair(sender.client_sock_, topic_msg + "\r\n");
	return (ret);
}

/****************************       <NO ** message>       ****************************/

/** @brief 403 - 존재하지 않는 channel을 parameter로 했을 때 발생하는 패킷 메세지 **/
Event	Sender::no_channel_message(const User& sender, const std::string& channel)
{
	Event			ret;
	std::string		no_msg;

	if (channel == "#")
	{
		no_msg = ":" + Sender::server_name_ + " 403 " + sender.nickname_ + " " + \
			channel + " " + ":No such const user in every channel";
	}
	else
	{
		no_msg = ":" + Sender::server_name_ + " 403 " + sender.nickname_ + " " + \
			channel + " " + ":No such channel";
	}
	ret = std::make_pair(sender.client_sock_, no_msg + "\r\n");
	return (ret);
}

/** @brief 401 - 존재하지 않는 target 클라이언트를 parameter로 사용하려 할 때 보내는 패킷 메시지 **/
Event	Sender::no_user_message(const User& sender, const std::string& target)
{
	Event		ret;

	const std::string& no_msg = ":" + Sender::server_name_ + " 401 " + sender.nickname_ + " " + \
		target + " " + ":No such const user";
	ret = std::make_pair(sender.client_sock_, no_msg + "\r\n");
	return (ret);
}