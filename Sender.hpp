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
		static udata pong(int usr_sock, std::string server_addr);		
		static udata welcome_message(int usr_sock, std::string server, std::string nick, std::string host);
		//static udata privmsg(struct user sender, struct user receiver, std::string msg);
		//static udata send_err(struct user sender, struct user receiver, std::string msg);
};

/*
 @brief PING명령에 대한 응답으로 PONG 전송s
 @param usr_sock 클라이언트 소켓
 @param server_addr 서버 주소
*/
udata Sender::pong(int usr_sock, std::string server_addr)
{
	udata ret;

	std::string pong_reply = ":" + server_addr + " PONG " \
		+ server_addr + " :" + server_addr + "\r\n"; 
	std::cout << RED << pong_reply << RESET << std::endl;

	ret.sock_fd = usr_sock;
	ret.msg = pong_reply;
	return ret;
}

udata Sender::welcome_message(int usr_sock, std::string server, std::string nick, std::string host)
{
	udata ret;

	std::string msg001 = ":" + server + " 001 " + nick +" Welcome to the 42's irc network " \
		+ nick + "!" + host + "\r\n";
	std::cout << usr_sock << " " << msg001 << std::endl;
	
	ret.sock_fd = usr_sock;
	ret.msg = msg001;
	return ret;
}

// udata Sender::privmsg(struct user sender, struct user receiver, std::string msg)
// {
// 	std::string privmsg;

// 	privmsg = ":" + sender.nickname_ + "@" + sender.servername_ + " PRIVMSG " + \
// 		receiver.nickname_ + " :" + msg + "\r\n";

// 	std::cout << "Message Sent : " << privmsg << std::endl;
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