#pragma once

#include "user.hpp"
#include "Udata.hpp"
#include <sys/types.h>
#include <sys/socket.h>
#include <iostream>
#include "color.hpp"

struct user;



class Sender 
{
	public:
		static Udata pong(struct user sender);		
		static Udata welcome_message_connect(struct user sender); // 아직 모름 -> 1번 바꿈
		static Udata nick_well_message(struct user sender, std::string new_nick);
		static Udata nick_error_message(struct user sender, std::string new_nick);
		static Udata quit_channel_message(struct user sender, std::string leave_message);
		static Udata quit_lobby_message(struct user sender, std::string leave_message);
		static Udata privmsg_message(struct user sender, std::string target, std::string msg);
		static Udata join_message(struct user sender, std::string channel);
		static Udata part_message(struct user sender, std::string channel);
		static Udata kick_message(struct user sender, std::string kicker, std::string subject, std::string channel);
		static Udata kick_error_not_op_message(struct user sender, std::string kicker, std::string channel);
		static Udata kick_error_no_user_message(struct user sender, std::string kicker, std::string subject, std::string channel);
		//static udata privmsg(struct user sender, struct user receiver, std::string msg);
		//static udata send_err(struct user sender, struct user receiver, std::string msg);
};

/****************************       <PING && PONG>       ****************************/

/*
 @brief PING명령에 대한 응답으로 PONG 전송s
 @param usr_sock 클라이언트 소켓
 @param sender.hostname_ 서버 주소
*/

static Udata pong(struct user sender) // 1st done
{
	Udata ret;

	std::string pong_reply = ":" + sender.hostname_ + " PONG " \
		+ sender.hostname_ + " :" + sender.hostname_ + "\r\n"; 

	ret.sock_fd = sender.client_sock_;
	ret.msg = pong_reply;
	return ret;
}

/****************************       <NICK>       ****************************/
/*
 @brief NICK명령 시 정상 적으로 작동
 @param user.event.indent user 소캣 
 @param nick 현재 닉네임
 @param user_name 유저 네임
 @param host 호스트네임
 @param new_nick 새로운 닉네임
*/
static Udata nick_well_message(struct user sender, std::string new_nick) // 1st done
{
	Udata	ret;

	std::string nick_msg = sender.nickname_ + "!" + sender.username_ \
					+ "@" + sender.hostname_ + " NICK :" + new_nick + "\r\n";
	ret.sock_fd = sender.client_sock_;
	ret.msg = nick_msg;
	return ret;
}

/*
 @brief NICK명령 시 중복일 경우 메세지
 @param user.event.indent user 소캣 
 @param host 서버 주소
 @param nick 현재 닉네임
 @param new_nick 새로운 닉네임
*/
static Udata nick_error_message(struct user sender, std::string new_nick) // 1st done
{
	Udata	ret;

	std::string nick_msg = sender.hostname_ + " 433 " + sender.nickname_ \
					+ " " + new_nick + "Nickname is already in use.\r\n";
	ret.sock_fd = sender.client_sock_;
	ret.msg = nick_msg;
	return ret;
}

/****************************       <Connect server || channel>       ****************************/

// :irc.local 001 root :Welcome to the Localnet IRC Network root!root@127.0.0.1
// Udata Sender::welcome_message_connect(int usr_sock, std::string server, std::string nick, std::string host)
static Udata welcome_message_connect(struct user sender) // 1st done
{
	Udata ret; 

	std::string msg001 = ":" + sender.hostname_ + " 001 " + sender.nickname_ \
	+ " :Welcome to the 42's irc network " + sender.nickname_ + "!" + sender.hostname_ + "\r\n";	
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
 @param user.event.indent user 소캣
 @param nick 현재 닉네임
 @param leave_message quit 할 때 사용자가 입력할 수도 있는 메세지
*/
static Udata quit_channel_message(struct user sender, std::string leave_message) // 1st done
{
	Udata	ret;

	if (leave_message.empty())
		leave_message = "leaving";
	std::string  quit_channel_message = "ERROR :Closing link: (" \
		+ sender.realname_ + ") [Quit: " + leave_message + "\r\n"; 
	ret.sock_fd = sender.client_sock_;
	ret.msg = quit_channel_message;
	return ret;
}
// @brief quit을 입력한 유저는 채팅방 속 다른 유저들과 다른 메세지를 호출함
static Udata quit_lobby_message(struct user sender, std::string leave_message) // 1st done
{
	Udata	ret;

	if (leave_message.empty())
		leave_message = "leaving";
	std::string  quit_lobby_message = ":" + sender.nickname_ + "! " \
				+ sender.realname_ + " QUIT :Quit: " + leave_message + "\r\n";
	ret.sock_fd = sender.client_sock_;
	ret.msg = quit_lobby_message;
	return ret;
}
/****************************       <PRIVMSG>       ****************************/

// 127.000.000.001.39548-127.000.000.001.06667: PRIVMSG #test :hihi

// 127.000.000.001.06667-127.000.000.001.39552: :junoh!root@127.0.0.1 PRIVMSG #test :hihi

// 127.000.000.001.06667-127.000.000.001.39546: :junoh!root@127.0.0.1 PRIVMSG #test :hihi

/*
 @brief 채팅 시 메세지
 @param user.event.indent user 소캣 
 @param nick 현재 닉네임
 @param user 유저네임
 @param target 메세지를 받는 클라이언트
 @param msg 유저가 입력한 메세지
*/
Udata privmsg_p2p(struct user sender, std::string target, std::string msg);


// 127.000.000.001.06667-127.000.000.001.39474: :hong!root@127.0.0.1 JOIN :#test
// [/join #] 로 명령어 입력 시 채널 방 이름은 #이다
/*
 @param user.event.indent user 소캣
 @brief join 시 메세지 
 @param nick 현재 닉네임
 @param user 유저네임
*/
static Udata join_message(struct user sender, std::string channel) // 1st->done
{
	Udata	ret;

	std::string  join_message = ":" + sender.nickname_ + "! " \
				+ sender.realname_ + " JOIN :#" + channel + "\r\n";
	ret.sock_fd = sender.client_sock_;
	ret.msg = join_message;
	return ret;
}

/****************************       <PART>       ****************************/

// 127.000.000.001.39544-127.000.000.001.06667: PART #test

// 127.000.000.001.06667-127.000.000.001.39544: :mypark!root@127.0.0.1 PART :#test

// 127.000.000.001.06667-127.000.000.001.39546: :mypark!root@127.0.0.1 PART :#test

// 127.000.000.001.06667-127.000.000.001.39548: :mypark!root@127.0.0.1 PART :#test

static Udata part_message(struct user sender, std::string channel) // 1st donw
{
	Udata	ret;

	std::string  part_message = ":" + sender.nickname_ + "!" \
				+ sender.realname_ + " PART :#" + channel + "\r\n";
	ret.sock_fd = sender.client_sock_;
	ret.msg = part_message;
	return ret;
}

/****************************       <KICK>       ****************************/

// 127.000.000.001.39548-127.000.000.001.06667: KICK #test mypark :

// 127.000.000.001.06667-127.000.000.001.39548: :junoh!root@127.0.0.1 KICK #test mypark :

// 127.000.000.001.06667-127.000.000.001.39552: :junoh!root@127.0.0.1 KICK #test mypark :

// 127.000.000.001.06667-127.000.000.001.39546: :junoh!root@127.0.0.1 KICK #test mypark :

static Udata kick_message(struct user sender, std::string kicker, std::string subject, std::string channel) // 1st done
{
	Udata	ret;

	std::string  kick_message = ":" + kicker + "!" + \
				sender.realname_ + " KICK #" + channel + " " + subject + "\r\n";
	ret.sock_fd = sender.client_sock_;
	ret.msg = kick_message;
	return ret;
}

//127.000.000.001.06667-127.000.000.001.39552: :irc.local 482 mypark #test :You must be a channel operator
static Udata kick_error_not_op_message(struct user sender, std::string kicker, std::string channel) // 1st done
{
	Udata	ret;

	std::string  kick_message = ":" + sender.hostname_ + \
		" 482 " + kicker + " #" + channel + " :You must be a channel operator\r\n";
	ret.sock_fd = sender.client_sock_;
	ret.msg = kick_message;
	return ret;	
}

//127.000.000.001.06667-127.000.000.001.39552: :irc.local 441 mypark junoh #ttt :They are not on that channel
static Udata kick_error_no_user_message(struct user sender, std::string kicker, std::string subject, std::string channel)
{
	Udata	ret;

	std::string  kick_message = ":" + sender.hostname_ + \
		" 441 " + kicker + " " + subject + " #" + channel + " :They are not on that channel\r\n";
	ret.sock_fd = sender.client_sock_;
	ret.msg = kick_message;
	return ret;	
}





// Udata Sender::privmsg(struct user sender, struct user receiver, std::string msg)
// {
// 	std::string privmsg;

// 	privmsg = ":" + sender.nickname_ + "@" + sender.servername_ + " PRIVMSG " + \
// 		receiver.nickname_ + " :" + msg + "\r\n";

// 	send(receiver.event.ident, privmsg.c_str(), privmsg.length(), 0);
// }

// sender : 보내려고 했던 대상, receiver : 잘못 닉네임을 입력해서 오류를 받아야 하는 대상
// udata Sender::send_err(struct user sender, struct user receiver, std::string msg)
// {
// 	std::string errmsg;

// 	errmsg = ":" + receiver.servername_ + " 401 " + receiver.nickname_ + " " + \
// 		sender.nickname_ + " :No such nick\r\n";
// 	send(receiver.event.ident, errmsg.c_str(), errmsg.length(), 0);
// }
