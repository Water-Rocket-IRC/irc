#pragma once

#include <sys/types.h>
#include <sys/socket.h>
#include <iostream>

class Sender
{
	public:
		static void pong(int usr_sock, std::string server_addr);
};

/*
 @brief PING명령에 대한 응답으로 PONG 전송
 @param usr_sock 클라이언트 소켓
 @param server_addr 서버 주소
*/
void Sender::pong(int usr_sock, std::string server_addr)
{
	std::string pong_reply = ":" + server_addr + " " + "PONG" + \
	" " + server_addr + " :" + server_addr;
	std::cout << pong_reply << std::endl;
	send(usr_sock, pong_reply.c_str(), pong_reply.length(), 0);
}