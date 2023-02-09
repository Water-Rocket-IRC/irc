#pragma once

#include "Receiver.hpp"

class Server
{
	private:
		Udata						serv_udata_;
		std::string					password_;
		uintptr_t					port_;
		static Receiver*			receiver_ptr_;

	public:
		static Server*				server_ptr_;
		Server(const std::string& port, const std::string& password);

		void	server_sigint(void);
		void	start();
		Udata&	get_server_udata(void);
};
