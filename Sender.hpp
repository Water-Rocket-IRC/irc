#pragma once

#include "Users.hpp"
#include <sys/types.h>
#include <sys/socket.h>
#include <iostream>

#include "color.hpp"

struct user;

class Sender
{
	public:
		static void pong(int usr_sock, std::string server_addr);
		static void welcome_message(int usr_sock, std::string server, std::string nick, std::string host);
		static void privmsg(user sender, user receiver, std::string msg);
		static void send_err(user sender, user receiver, std::string msg);
};

/*
 @brief PING명령에 대한 응답으로 PONG 전송
 @param usr_sock 클라이언트 소켓
 @param server_addr 서버 주소
*/
void Sender::pong(int usr_sock, std::string server_addr)
{
	std::string pong_reply = ":" + server_addr + " PONG " \
		+ server_addr + " :" + server_addr + "\r\n"; 
	std::cout << RED << pong_reply << RESET << std::endl;
	send(usr_sock, pong_reply.c_str(), pong_reply.length(), 0);
}

void Sender::welcome_message(int usr_sock, std::string server, std::string nick, std::string host)
{
	std::string msg001 = ":" + server + " 001 " + nick +" Welcome to the 42's irc network " \
		+ nick + "!" + host + "\r\n";
	std::cout << usr_sock << " " << msg001 << std::endl;
	send(usr_sock, msg001.c_str(), msg001.length(), 0);
}

void Sender::privmsg(user sender, user receiver, std::string msg)
{
	std::string privmsg;

	privmsg = ":" + sender.nickname_ + "@" + sender.servername_ + " PRIVMSG " + \
		receiver.nickname_ + " :" + msg + "\r\n";

	std::cout << "Message Sent : " << privmsg << std::endl;
	send(receiver.event.ident, privmsg.c_str(), privmsg.length(), 0);
}

// sender : 보내려고 했던 대상, receiver : 잘못 닉네임을 입력해서 오류를 받아야 하는 대상
void Sender::send_err(user sender, user receiver, std::string msg)
{
	std::string errmsg;

	errmsg = ":" + receiver.servername_ + " 401 " + receiver.nickname_ + " " + \
		sender.nickname_ + " :No such nick\r\n";
	send(receiver.event.ident, errmsg.c_str(), errmsg.length(), 0);
}