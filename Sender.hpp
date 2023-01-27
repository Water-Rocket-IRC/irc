#pragma once

#include "Users.hpp"
#include <sys/types.h>
#include <sys/socket.h>
#include <iostream>

#include "color.hpp"

class Sender
{
	public:
		static void pong(int usr_sock, std::string server_addr);
		static void welcome_message(int usr_sock, std::string server, std::string nick, std::string host);
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
