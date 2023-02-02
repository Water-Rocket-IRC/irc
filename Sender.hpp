#ifndef SENDER_HPP
# define SENDER_HPP

#include "user.hpp"
#include "Udata.hpp"
#include <sys/types.h>
#include <sys/socket.h>
#include <iostream>
#include "color.hpp"

// struct user;

class Sender
{ 
	public:
		static Udata	unknown_command_message_421(const user& sender, const std::string command);
		static Udata	pong(uintptr_t socket, const std::string& serv_addr);
		static Udata	command_no_origin_specified_409(const user& sender, const std::string command);
		static Udata	command_not_registered_451(const user& sender, const std::string command);
		static Udata	command_not_registered_451(const uintptr_t& sock, const std::string command);
		static Udata	command_empty_argument_461(const user& sender, const std::string command);
		static Udata	command_empty_argument_461(const uintptr_t& sock, const std::string command);
		static Udata	welcome_message_connect(const user& sender); // 아직 모름 -> 1번 바꿈
		static Udata	nick_well_message(const user& sender, const user& receiver, const std::string& new_nick);
		static Udata	nick_error_message(const user& sender, const std::string& new_nick);
		static Udata	nick_error_message(const std::string& new_nick, uintptr_t& sock);
		static Udata	nick_wrong_message(const user& sender, const std::string& new_nick);
		static Udata	quit_channel_message(const user& sender, const user& receiver, std::string leave_message);
		static Udata	quit_lobby_message(const user& sender, std::string leave_message);
		static Udata	privmsg_p2p_message(const user& sender, const user& target, const std::string& msg);
		static Udata	privmsg_channel_message(const user& sender, const user& receiver, const std::string& channel, const std::string& msg);
		static Udata	privmsg_no_user_error_message(const user& sender, const std::string& target);
		static Udata	privmsg_external_error_message(const user& sender, const std::string channel);
		static Udata	join_message(const user& sender, const user& receiver, const std::string& channel);
		static Udata	join_invaild_channel_name_message(const user& sender, const std::string invaild_channel);
		static Udata	part_message(const user& sender, const user& receiver, const std::string& channel, const std::string& msg);
		static Udata	kick_message(const user& sender, const user& receiver, const std::string& subject, const std::string& channel, const std::string& msg);
		static Udata	kick_error_not_op_message(const user& sender, const std::string& host, const std::string& channel);
		static Udata	kick_error_no_user_message(const user& sender, const std::string& host, const std::string& subject, const std::string& channel);
		static Udata	topic_message(const user& sender, const user& receiver, const std::string& channel, const std::string& topic);
		static Udata	topic_error_message(const user& sender, const std::string& channel);
		static Udata	notice_p2p_message(const user& sender, const user& target, const std::string& msg);
		static Udata	notice_channel_message(const user& sender, const user& receiver, const std::string& channel, const std::string& msg);
		static Udata 	notice_no_nick_message(const user& sender, const user& receiver);
		static Udata	wall_message(const user& sender, const user& receiver, const std::string& channel, const std::string& msg);
		static Udata	no_channel_message(const user& sender, const std::string& channel);
		static Udata	no_user_message(const user& sender, const std::string& target);
		static Udata	mode_324_message(const user& sender, const std::string channel);
		static Udata	mode_no_option_error_message(const user& sender, const std::string wrong_option);
		static Udata	mode_not_operator_error_message(const user& sender, const std::string channel);
		static Udata	who_joiner_352_message(const user& sender, const std::string channel);
		static std::string	who_352_target_message(const user& sender, const std::string channel, const std::string target);
		static std::string	who_315_message(const user& sender, const std::string channel);
		static std::string	mode_329_message(const user& sender, const std::string channel, const std::string time_stamp);
		static std::string	join_353_message(const user& sender, const std::string& chan_name, const std::string& chan_status, const std::string& chan_user_list);
		static std::string	join_366_message(const user& sender, const std::string& chan_name);
	private:
		static const std::string	server_name_;
};

const std::string	Sender::server_name_ = "webserv.local";
/****************************       <PING && PONG && USE && etc>       ****************************/

/*
 @brief PING명령에 대한 응답으로 PONG 전송s
 @param usr_sock 클라이언트 소켓
 @param sender.hostname_ 서버 주소
*/

Udata	Sender::pong(uintptr_t socket, const std::string& serv_addr) // 1st done
{
	Udata ret;

	std::string pong_reply = ":" + serv_addr + " PONG " \
		+ serv_addr + " :" + serv_addr + "\r\n";
	ret.sock_fd = socket;
	ret.msg = pong_reply;
	return ret;
}

Udata	Sender::command_empty_argument_461(const user& sender, const std::string command)
{
	Udata ret;

	std::string error_message = ":" + Sender::server_name_ + " 461 " \
		+ sender.nickname_ + " " + command  + " :Not enough parameters\r\n";
	ret.sock_fd = sender.client_sock_;
	ret.msg = error_message;
	return ret;
}

Udata	Sender::command_empty_argument_461(const uintptr_t& sock, const std::string command)
{
	Udata ret;

	std::string error_message = ":" + Sender::server_name_ + " 461 * " + command  + " :Not enough parameters\r\n";
	ret.sock_fd = sock;
	ret.msg = error_message;
	return ret;
}

Udata	Sender::command_not_registered_451(const user& sender, const std::string command)
{
	Udata ret;

	std::string error_message = ":" + Sender::server_name_ + " 451 " \
		+ sender.nickname_ + " " + command  + " :You have not registered\r\n";
	ret.sock_fd = sender.client_sock_;
	ret.msg = error_message;
	return ret;
}

Udata	Sender::command_not_registered_451(const uintptr_t& sock, const std::string command)
{
	Udata ret;

	std::string error_message = ":" + Sender::server_name_ + " 451 * " \
		+ " " + command  + " :You have not registered\r\n";
	ret.sock_fd = sock;
	ret.msg = error_message;
	return ret;
}

Udata	Sender::command_no_origin_specified_409(const user& sender, const std::string command)
{
	Udata ret;

	std::string error_message = ":" + Sender::server_name_ + " 409 " \
		+ sender.nickname_ + " " + command  + " :No origin specified\r\n";
	ret.sock_fd = sender.client_sock_;
	ret.msg = error_message;
	return ret;
}

Udata	Sender::unknown_command_message_421(const user& sender, const std::string command)
{
	Udata ret;

	std::string error_message = ":" + Sender::server_name_ + " 421 " \
		+ sender.nickname_ + " " + command  + " :Unknown command\r\n";
	ret.sock_fd = sender.client_sock_;
	ret.msg = error_message;
	return ret;	
}
/****************************       <NICK>       ****************************/
/*
 @brief NICK명령 시 정상 적으로 작동
 @param const user.event.indent const user& 소캣 
 @param nick 현재 닉네임
 @param user_name 유저 네임
 @param host 호스트네임
 @param new_nick 새로운 닉네임
*/
Udata	Sender::nick_well_message(const user& sender, const user& receiver, const std::string& new_nick) // 1st done
{
	Udata	ret;

	std::string nick_msg = ":" + sender.nickname_ + "!" + sender.username_ \
					+ "@" + sender.unused_ + " NICK :" + new_nick + "\r\n";
	ret.sock_fd = receiver.client_sock_;
	ret.msg = nick_msg;
	return ret;
}

/*
 @brief NICK명령 시 중복일 경우 메세지
 @param const user.event.indent const user 소캣 
 @param host 서버 주소
 @param nick 현재 닉네임
 @param new_nick 새로운 닉네임
*/
Udata	Sender::nick_error_message(const user& sender, const std::string& new_nick) // 1st done
{
	Udata	ret;

	std::string nick_msg = ":" + Sender::server_name_ + " 433 " + sender.nickname_ \
					+ " " + new_nick + " :Nickname is already in use.\r\n";
	ret.sock_fd = sender.client_sock_;
	ret.msg = nick_msg;
	return ret;
}

Udata	Sender::nick_error_message(const std::string& new_nick, uintptr_t& sock)
{
	Udata	ret;

	std::string nick_msg = ":" + Sender::server_name_ + " 433 * " + new_nick + " :Nickname is already in use.\r\n";
	ret.sock_fd = sock;
	ret.msg = nick_msg;
	return ret;
}

Udata	Sender::nick_wrong_message(const user& sender, const std::string& new_nick)
{
	Udata	ret;

	std::string nick_msg = ":" + Sender::server_name_ + " 432 " + sender.nickname_ \
					+ " " + new_nick + "Erroneous Nickname.\r\n";
	ret.sock_fd = sender.client_sock_;
	ret.msg = nick_msg;
	return ret;
}
/****************************       <Connect server || channel>       ****************************/

// :irc.local 001 root :Welcome to the Localnet IRC Network root!root@127.0.0.1
// Udata Sender::welcome_message_connect(int usr_sock, const std::string& server, const std::string& nick, const std::string& host)
Udata	Sender::welcome_message_connect(const user& sender) // 1st done
{
	Udata ret; 

	std::string msg001 = ":" + Sender::server_name_ + " 001 " + sender.nickname_ \
	+ " :Welcome to the 42's irc network " + sender.nickname_ + "!" + sender.mode_ + "\r\n";	
	ret.sock_fd = sender.client_sock_;
	ret.msg = msg001;
	return ret;
}

/****************************       <QUIT>       ****************************/

// 127.000.000.001.06667-127.000.000.001.39470: ERROR :Closing link: (root@127.0.0.1) [Quit: byby]

// 127.000.000.001.06667-127.000.000.001.39460: :root_!root@127.0.0.1 QUIT :Quit: byby

// 127.000.000.001.06667-127.000.000.001.39466: :root_!root@127.0.0.1 QUIT :Quit: byby

/*
 @brief quit 명령 시 메세지 - quit을 한 유저가 아닌 다른 유저들 
 @param const user.event.indent const user& 소캣
 @param nick 현재 닉네임
 @param leave_message quit 할 때 사용자가 입력할 수도 있는 메세지
*/
Udata	Sender::quit_channel_message(const user& sender, const user& receiver, std::string leave_message) // 2st done
{
	Udata	ret;

	if (leave_message.empty())
		leave_message = "leaving";
	std::string  quit_channel_message = "ERROR :Closing link: (" \
		+ sender.realname_ + ") [Quit: " + leave_message + "\r\n"; 
	ret.sock_fd = receiver.client_sock_;
	ret.msg = quit_channel_message;
	return ret;
}
// @brief quit을 입력한 유저는 채팅방 속 다른 유저들과 다른 메세지를 호출함
Udata	Sender::quit_lobby_message(const user& sender, std::string leave_message) // 2st done
{
	Udata	ret;

	if (leave_message.empty())
		leave_message = "leaving";
	std::string  quit_message = ":" + sender.nickname_ + "! " \
				+ sender.realname_ + " QUIT :Quit: " + leave_message + "\r\n";
	ret.sock_fd = sender.client_sock_;
	ret.msg = quit_message;
	return ret;
}
/****************************       <JOIN>       ****************************/

// 127.000.000.001.39548-127.000.000.001.06667: PRIVMSG #test :hihi

// 127.000.000.001.06667-127.000.000.001.39552: :junoh!root@127.0.0.1 PRIVMSG #test :hihi

// 127.000.000.001.06667-127.000.000.001.39546: :junoh!root@127.0.0.1 PRIVMSG #test :hihi

/*
 @brief 채팅 시 메세지
 @param const user.event.indent const user 소캣 
 @param nick 현재 닉네임
 @param const user 유저네임
 @param target 메세지를 받는 클라이언트
 @param msg 유저가 입력한 메세지
*/
// Udata	privmsg_p2p(const user sender, const std::string& target, const std::string msg);


// 127.000.000.001.06667-127.000.000.001.39474: :hong!root@127.0.0.1 JOIN :#test
// [/join #] 로 명령어 입력 시 채널 방 이름은 #이다
/*
 @param const user.event.indent const user 소캣
 @brief join 시 메세지
 @param nick 현재 닉네임
 @param const user 유저네임
*/
Udata	Sender::join_message(const user& sender, const user& receiver, const std::string& channel) // 2st->done
{
	Udata	ret;

	std::string  join_message = ":" + sender.nickname_ + "!" \
				+ sender.realname_ + "@" + sender.unused_ + " JOIN " + channel + "\r\n";
	ret.sock_fd = receiver.client_sock_;//receciver의 ident
	ret.msg = join_message;
	return ret;
}

Udata	Sender::join_invaild_channel_name_message(const user& sender, const std::string invaild_channel)
{
	Udata ret;

	std::string error_message = ":" + Sender::server_name_ + " 476 " \
		+ sender.nickname_ + " " + invaild_channel  + " :Invaild channel name\r\n";
	ret.sock_fd = sender.client_sock_;
	ret.msg = error_message;
	return ret;	
}

/*
@brief RPL_NAMREPLY(353)
@param 1. sender	보내는 사람
	   2. const user 유저네임
	   3. channel_status
		ㄴ '=' : public channel
    	ㄴ '*' : private channel
    	ㄴ '@' : secret channel
@example ":irc.local 353 one = #123 :one two"
*/
std::string	Sender::join_353_message(const user& sender, const std::string& chan_name, const std::string& chan_access, const std::string& chan_user_list)
{
	std::string  ret = ":" + Sender::server_name_ + " 353 "+ sender.nickname_
				+ " " + chan_access + " " + chan_name + " :" + chan_user_list + "\r\n";
	return ret;
}


/*
@brief RPL_NAMREPLY(353)
@param 1. sender	보내는 사람
	   2. const user 유저네임
	   3. channel_status
		ㄴ '=' : public channel
    	ㄴ '*' : private channel
    	ㄴ '@' : secret channel
@example ":irc.local 366 two #123 :End of /NAMES list."
*/
std::string	Sender::join_366_message(const user& sender, const std::string& chan_name)
{
	std::string  ret = ":" + Sender::server_name_ + " 366 "+ sender.nickname_
				+ " " + chan_name + " :End of /NAMES list.\r\n";
	return ret;
}

// 127.000.000.001.45062-127.000.000.001.06667: JOIN #test

// 127.000.000.001.06667-127.000.000.001.45062: :root!root@127.0.0.1 JOIN :#test
// :irc.local 353 root = #test :me hong root
// :irc.local 366 root #test :End of /NAMES list.

// 127.000.000.001.06667-127.000.000.001.39562: :root!root@127.0.0.1 JOIN :#test

// 127.000.000.001.06667-127.000.000.001.44832: :root!root@127.0.0.1 JOIN :#test

// 127.000.000.001.45062-127.000.000.001.06667: MODE #test

// 127.000.000.001.06667-127.000.000.001.45062: :irc.local 324 root #test :+nt
// :irc.local 329 root #test :1675311260

// 127.000.000.001.45062-127.000.000.001.06667: WHO #test

// 127.000.000.001.06667-127.000.000.001.45062: :irc.local 352 root #test root 127.0.0.1 irc.local me H :0 root
// :irc.local 352 root #test root 127.0.0.1 irc.local hong H :0 root
// :irc.local 352 root #test root 127.0.0.1 irc.local root H :0 root
// :irc.local 315 root #test :End of /WHO list.

// 127.000.000.001.45062-127.000.000.001.06667: MODE #test b

// 127.000.000.001.06667-127.000.000.001.45062: :irc.local 368 root #test :End of channel ban list

Udata	Sender::mode_324_message(const user& sender, const std::string channel)
{
	Udata	ret;

	std::string  mode_message = ":" + Sender::server_name_ \
			+ " 324 " + sender.nickname_ + " " + channel + ":+nt\r\n";

	ret.sock_fd = sender.client_sock_;
	ret.msg = mode_message;
	return ret;
}

std::string	Sender::mode_329_message(const user& sender, const std::string channel, const std::string time_stamp)
{
	std::string ret = ":" + Sender::server_name_ + " 329 " \
			+ sender.nickname_ + " " + channel + " :" + time_stamp + "\r\n";
	return ret;
}

Udata	Sender::mode_no_option_error_message(const user& sender, const std::string channel)
{
	Udata ret;

	std::string mode_message = ":" + Sender::server_name_ + " 472 " + sender.nickname_ \
		+ " " +channel + " :You must have channel op access or above to set channel mode p\r\n"; 
	ret.sock_fd = sender.client_sock_;
	ret.msg = mode_message;
	return ret;
}

Udata	Sender::mode_not_operator_error_message(const user& sender, const std::string wrong_option)
{
	Udata ret;

	std::string mode_message = ":" + Sender::server_name_ + " 482 " + sender.nickname_ \
		+ " " + wrong_option + " :is not a recognised channel mode\r\n"; 
	ret.sock_fd = sender.client_sock_;
	ret.msg = mode_message;
	return ret;
}
/* 
 @brief join 할때 joiner(조인 하는 자신)에게도 메세지가 보내짐.
*/
Udata	Sender::who_joiner_352_message(const user& sender, const std::string channel)
{
	Udata ret;

	std::string who_message = ":" + Sender::server_name_ + " 352 " + sender.nickname_ \
			+ " " + channel + " " + sender.username_ + " " + sender.mode_ + " " \
			+ sender.unused_ + " " + sender.nickname_ + " H :0 root\r\n";
	ret.sock_fd = sender.client_sock_;
	ret.msg = who_message;
	return ret;
}
/*
 @brief who_joiner_352_message 함수 호출 직후에 호출되는 함수
*/ 
std::string	Sender::who_352_target_message(const user& sender, const std::string channel, const std::string target)
{
	std::string ret = ":" + Sender::server_name_ + " 352 " + sender.nickname_ \
			+ " " + channel + " " + sender.username_ + " " + sender.mode_ + " " \
			+ sender.unused_ + " " + target + " H :0 root\r\n";
	return ret;
}

/*
 @brief who_352_target_message 함수 호출 직후에 호출되는 함수
*/
std::string	Sender::who_315_message(const user& sender, const std::string channel)
{
	std::string ret = ":" + Sender::server_name_ + " 315 " + sender.nickname_ \
		+ " " + channel + " :End of /WHO list\r\n";
	return ret;
}

/****************************       <PART>       ****************************/

// 127.000.000.001.39544-127.000.000.001.06667: PART #test

// 127.000.000.001.06667-127.000.000.001.39544: :mypark!root@127.0.0.1 PART :#test

// 127.000.000.001.06667-127.000.000.001.39546: :mypark!root@127.0.0.1 PART :#test

// 127.000.000.001.06667-127.000.000.001.39548: :mypark!root@127.0.0.1 PART :#test

Udata	Sender::part_message(const user& sender, const user& receiver, const std::string& channel, const std::string& msg) // 2st done
{
	Udata	ret;

	std::string  part_message = ":" + sender.nickname_ + "!" \
				+ sender.username_ + "@" + receiver.mode_ + " PART " + channel + " " + msg + "\r\n";
	ret.sock_fd = receiver.client_sock_;
	ret.msg = part_message;
	return ret;
}

/****************************       <KICK>       ****************************/

// 127.000.000.001.39548-127.000.000.001.06667: KICK #test mypark :

// 127.000.000.001.06667-127.000.000.001.39548: :junoh!root@127.0.0.1 KICK #test mypark :

// 127.000.000.001.06667-127.000.000.001.39552: :junoh!root@127.0.0.1 KICK #test mypark :

// 127.000.000.001.06667-127.000.000.001.39546: :junoh!root@127.0.0.1 KICK #test mypark :

Udata	Sender::kick_message(const user& sender, const user& receiver, const std::string& target, const std::string& channel, const std::string& msg) // 1st done
{
	Udata	ret;

	std::string  kick_message = ":" + sender.nickname_ + "!" + \
				sender.realname_ + '@' + sender.unused_ + " KICK " + channel + " " + target + " :" + msg + "\r\n";
	// const std::string  kick_message = ":" + host + "!" + \
	// 			sender.realname_ + " KICK " + channel + " " + subject + "\r\n";
	ret.sock_fd = receiver.client_sock_;
	ret.msg = kick_message;
	return ret;
}

//127.000.000.001.06667-127.000.000.001.39552: :irc.local 482 mypark #test :You must be a channel operator
Udata	Sender::kick_error_not_op_message(const user& sender, const std::string& host, const std::string& channel) // 1st done
{
	Udata	ret;

	std::string  kick_message = ":" + Sender::server_name_ + \
		" 482 " + host + " " + channel + " You must be a channel operator\r\n";
	ret.sock_fd = sender.client_sock_;
	ret.msg = kick_message;
	return ret;	
}

//127.000.000.001.06667-127.000.000.001.39552: :irc.local 441 mypark junoh #ttt :They are not on that channel
Udata	Sender::kick_error_no_user_message(const user& sender, const std::string& host, const std::string& subject, const std::string& channel)
{
	Udata	ret;

	std::string  kick_message = ":" + Sender::server_name_ + \
		" 441 " + host + " " + subject + " " + channel + " :They are not on that channel\r\n";
	ret.sock_fd = sender.client_sock_;
	ret.msg = kick_message;
	return ret;
}

/****************************       <Privmsg>       ****************************/
Udata	Sender::privmsg_p2p_message(const user& sender, const user& target, const std::string& msg) //2st done
{
	std::string privmsg;
	Udata		ret;

	privmsg = ":" + sender.nickname_ + "@" + sender.unused_ + " PRIVMSG " + \
		target.nickname_ + " :" + msg + "\r\n";

	ret.sock_fd = target.client_sock_;
	ret.msg = privmsg;
	return (ret);
}

Udata	Sender::privmsg_channel_message(const user& sender, const user& receiver, const std::string& msg, const std::string& channel) // 2st done
{
	Udata		ret;

	std::string privmsg = ":" + sender.nickname_ + "@" + sender.unused_ + " PRIVMSG " + \
		channel + " :" + msg + "\r\n";

	ret.sock_fd = receiver.client_sock_;
	ret.msg = privmsg;
	return (ret);
}

Udata	Sender::privmsg_no_user_error_message(const user& sender, const std::string& target)
{
	Udata		ret;

	std::string privmsg = ":" + sender.nickname_ + " 401 " + sender.nickname_ + " " + target \
	+ " :No such nick\r\n";

	ret.sock_fd = sender.client_sock_;
	ret.msg = privmsg;
	return (ret);
}

Udata	Sender::privmsg_external_error_message(const user& sender, const std::string channel)
{
	Udata		ret;

	std::string privmsg = ":" + sender.nickname_ + " 404 " + sender.nickname_ + " " + channel \
	+ " :You cannot send external messages to this channel whilist the +n (noextmsg) mode is set\r\n";

	ret.sock_fd = sender.client_sock_;
	ret.msg = privmsg;
	return (ret);
}

/****************************       <NOTICE && WALL>       ****************************/
Udata	Sender::notice_p2p_message(const user& sender, const user& target, const std::string& msg) //2st done
{
	Udata		ret;

	std::string privmsg = ":" + sender.nickname_ + "@" + sender.unused_ + " NOTICE " + \
		target.nickname_ + " :" + msg + "\r\n";

	ret.sock_fd = target.client_sock_;
	ret.msg = privmsg;
	return (ret);
}

Udata	Sender::notice_channel_message(const user& sender, const user& receiver, const std::string& msg, const std::string& channel) // 2st done
{
	Udata		ret;

	std::string privmsg = ":" + sender.nickname_ + "@" + sender.unused_ + " NOTICE " + \
		channel + " :" + msg + "\r\n";

	ret.sock_fd = receiver.client_sock_;
	ret.msg = privmsg;
	return (ret);
}

Udata 	Sender::notice_no_nick_message(const user& sender, const user& receiver)
{
	Udata		ret;

	std::string privmsg = ":" + Sender::server_name_ + " 401 " + sender.nickname_ + " " + receiver.username_ + \
	" No such nick\r\n";

	ret.sock_fd = sender.client_sock_;
	ret.msg = privmsg;
	return (ret);
}

Udata	Sender::wall_message(const user& sender, const user& receiver, const std::string& channel, const std::string& msg)
{
	Udata		ret;

	std::string privmsg = ":" + sender.nickname_ + "!" + sender.username_ + "@" + receiver.unused_ + \
	" NOTICE @" + channel + " :" + msg + "\r\n";
	ret.sock_fd = receiver.client_sock_;
	ret.msg = privmsg;
	return (ret);
}
/****************************       <TOPIC>       ****************************/
Udata	Sender::topic_message(const user& sender, const user& receiver, const std::string& channel, const std::string& topic) //2st done
{
	Udata		ret;

	std::string topic_msg = ":" + sender.unused_ + "!" + sender.username_ + " TOPIC " + \
		channel + " " + topic + "\r\n";

	ret.sock_fd = receiver.client_sock_;
	ret.msg = topic_msg;
	return (ret);
}

Udata	Sender::topic_error_message(const user& sender, const std::string& channel) // 2st done
{
	Udata		ret;

	std::string topic_msg = ":" + Sender::server_name_ + " 482 " + sender.nickname_ + " " + \
		channel + " " + "You do not have access to change the topic on this channel"  + "\r\n";

	ret.sock_fd = sender.client_sock_;
	ret.msg = topic_msg;
	return (ret);
}

/****************************       <NO ** message>       ****************************/
//127.000.000.001.06667-127.000.000.001.59898: :irc.local 403 two #111 :No such channel

Udata	Sender::no_channel_message(const user& sender, const std::string& channel)
{
	Udata		ret;
	std::string no_msg;

	if (channel == "#")
	{
		no_msg = ":" + Sender::server_name_ + " 403 " + sender.nickname_ + " " + \
			channel + " " + ":No such const user in every channel"  + "\r\n";
	}
	else
	{
		no_msg = ":" + Sender::server_name_ + " 403 " + sender.nickname_ + " " + \
			channel + " " + ":No such channel"  + "\r\n";
	}
	ret.sock_fd = sender.client_sock_;
	ret.msg = no_msg;
	return (ret);
}

//127.000.000.001.06667-127.000.000.001.59898: :irc.local 401 junoh dd :No such nick
Udata	Sender::no_user_message(const user& sender, const std::string& target)
{
	Udata		ret;

	std::string no_msg = ":" + Sender::server_name_ + " 401 " + sender.nickname_ + " " + \
		target + " " + ":No such const user"  + "\r\n";

	ret.sock_fd = sender.client_sock_;
	ret.msg = no_msg;
	return (ret);
}
// sender : 보내려고 했던 대상, receiver : 잘못 닉네임을 입력해서 오류를 받아야 하는 대상
// Udata	Sender::send_err(const user sender, const user receiver, const std::string msg)
// {
// 	const std::string errmsg;

// 	errmsg = ":" + receiver.servername_ + " 401 " + receiver.nickname_ + " " + \
// 		sender.nickname_ + " :No such nick\r\n";
// 	send(receiver.client_sock_, errmsg.c_str(), errmsg.length(), 0);
// }

#endif