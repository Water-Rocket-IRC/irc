#pragma once

#include <sys/socket.h>
#include <sys/event.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include "KeventHandler.hpp"
#include "Parser.hpp"

// class Users;
// class Channels;
// class Udata;
class Parser;

// TODO: have to tommorow
// USER 클래스를 Parser로 이동
// std::vector<Udata>를 server에서 생성 후 Receiver에서 레퍼런스로 받아오면 좋을듯
// >> Parser도 std::vector<Udata>를 레퍼런스로 받아오면 서로 참조가 가능하니 좋을듯?
// ***생각해보니 Receiver 에서 Parser를 가져야된다. 아니면 Parser::command_parser함수를 못 부른다.***

class Receiver
{
	private:
		static KeventHandler	kq_;
		Parser					parser_;
		Udata					udata_;
		sockaddr_in				server_addr_;
		const uintptr_t			port_;
		uintptr_t				server_sock_;

		int						clientReadEventHandler_(struct kevent &cur_event);
		void					clientWriteEventHandler_(struct kevent &cur_event);

		void					init_socket_(const uintptr_t &port);
		void					bind_socket_();

	public:
		Receiver(Udata& serv_udata, const uintptr_t& port, const std::string& password);
		~Receiver();

		void 					start();
		static KeventHandler	get_Kevent_Handler();
};
