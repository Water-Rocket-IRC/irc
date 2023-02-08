#pragma once

#include <sys/socket.h>
#include <sys/event.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include "KeventHandler.hpp"
#include "Parser.hpp"

class Parser;

class Receiver
{
	private:
		static KeventHandler	kq_;
		Parser					parser_;
		Udata&					udata_;
		sockaddr_in				server_addr_;
		const uintptr_t			port_;
		uintptr_t				server_sock_;

		int						clientReadEventHandler_(struct kevent &cur_event);
		int						clientWriteEventHandler_(struct kevent &cur_event);

		void					init_socket_(const uintptr_t &port);
		void					bind_socket_();

	public:
		Receiver(Udata& serv_udata, const uintptr_t& port, const std::string& password);
		~Receiver();

		void 					start();
		static KeventHandler&	get_Kevent_Handler();
};
