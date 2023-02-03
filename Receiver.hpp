#ifndef RECEIVER_HPP
# define RECEIVER_HPP

#include <sys/socket.h>
#include <sys/event.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include "KeventHandler.hpp"
#include "Users.hpp"
#include "Channels.hpp"
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
		sockaddr_in				server_addr_;
		Udata					udata_;
		std::string				port_;
		std::string				password_;
		uintptr_t				server_sock_;
		uintptr_t				client_sock_;

		int						clientReadEventHandler_(struct kevent &cur_event);
		int						clientWriteEventHandler_(struct kevent &cur_event);

		void					init_socket_(int &port);
		void					bind_socket_();

	public:
		Receiver(int port, Udata& serv_udata);
		~Receiver();

		void 					start();
		static KeventHandler	get_Kevent_Handler();
};

#endif 
