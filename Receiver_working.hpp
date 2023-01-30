#pragma once

#include "KeventHandler.hpp"
#include "Users.hpp"

#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/event.h>

#include <sys/types.h>
#include <netdb.h>

class Receiver
{
	class SocketCreateFail : public std::exception
	{
		public :
			const char*	what() const throw();
	};
	class SocketBindFail : public std::exception
	{
		public :
			const char*	what() const throw();
	};
	class KqueueCreateFail : public std::exception
	{
		public :
			const char*	what() const throw();
	};

	private:
		KeventHandler	kq_;
		std::vector<struct kevent> events_;
		sockaddr_in		server_addr_;
		int				client_sock_;
		int				server_sock_;
		std::string		port_;
		std::string		password_;

	public:
		Users	Users;
		Receiver(int port);
		~Receiver();
		void	initSocket(int &port);
		void	bindSocket();
		void 	start();
		int		clientReadEventHandler(struct kevent &cur_event);
		int		clientWriteEventHandler(struct kevent &cur_event);

		user who_is_sender(struct kevent event);

};

/*    Throw Class     */
// const char*	Receiver::SocketCreateFail::what() const throw()
// {
// 	system("clear");
// 	return ("err: Socket creating fail");
// }

// const char*	Receiver::SocketBindFail::what() const throw()
// {
// 	system("clear");
// 	return ("err: Socket binding fail");
// }

// const char*	Receiver::KqueueCreateFail::what() const throw()
// {
// 	system("clear");
// 	return ("err: Kqueue creating fail");
// }

/*    Receiver Class     */
/// @brief Receiver 생성자
/// @param port 소켓을 생성할 포켓 번호
Receiver::Receiver(int port)
{
	initSocket(port);
	bindSocket();
}

void	Receiver::initSocket(int &port)
{
	server_sock_ = socket(AF_INET, SOCK_STREAM, 0);
	if (server_sock_ < 0)
		exit_with_perror("err: Socket Creating Fail");
		// throw SocketCreateFail();
	server_addr_.sin_family = AF_INET;
	server_addr_.sin_port = htons(port);
	server_addr_.sin_addr.s_addr = INADDR_ANY;
}

Receiver::~Receiver()
{
}

void Receiver::bindSocket()
{
	// socket bind
	if (bind(server_sock_, (sockaddr *) &server_addr_, sizeof(server_addr_)) < 0)
		exit_with_perror("err: Socket Binding Fail");
		// throw SocketBindFail();
	if (listen(server_sock_, 5) < 0) {
		std::cerr << "error" << std::endl;
	} // TODO: Have to arrange 5 (Max queue)

	kq_.SetRead(server_sock_, 0);
}

void hi() {
	std::cout << "hi" << std::endl;
}

void Receiver::start()
{
	while (true)
	{
		events_ = kq_.SetMonitor();
		for (size_t i(0); i < events_.size(); ++i)
		{
			struct kevent	cur_event = events_[i];	// event occur with new accept
			if (cur_event.ident == server_sock_)
			{
				client_sock_ = accept(server_sock_, NULL, NULL);
				if (client_sock_ < 0)
				{
					std::cerr << "err: accepting connection fail" << std::endl;
					continue ;
				}
				kq_.SetRead(client_sock_, 0); // ADD udata or not
			}
			else	// event occur with users
			{
				if (cur_event.filter == EVFILT_READ)
				{
					if (clientReadEventHandler(cur_event))
					{
						continue ;
					}
				}
				else if (cur_event.filter == EVFILT_WRITE)
				{
					// TODO: LEAK CHECK
					if (clientWriteEventHandler(cur_event))
					{
						Udata	*tmp = reinterpret_cast<Udata *>(cur_event.udata);
						int	tmp_fd = cur_event.ident;
						kq_.DeleteEvent(cur_event);
						delete tmp;
						kq_.SetRead(tmp_fd, 0);
					}
				}
			}
		}
	}
}

int	Receiver::clientReadEventHandler(struct kevent &cur_event)
{
	char buffer[512];


	memset(buffer, 0, sizeof(buffer));
	// TODO: LEAK CHECK
	if (cur_event.flags & EV_EOF)
	{
		std::cout << "sock was fucked!" << std::endl;
		Udata	*tmp = reinterpret_cast<Udata *>(cur_event.udata);
		kq_.DeleteEvent(cur_event);
		delete tmp;
	}
	int byte_received = recv(cur_event.ident, buffer, sizeof(buffer), 0);
	if (byte_received < 0)
	{
		std::cerr << "err: receiving data" << std::endl;
		return (1);
	}
	std::cout << "Received: " << buffer << std::endl;
	std::string			command(buffer, byte_received);
	std::stringstream	ss(command);
	std::string			line;

	while (std::getline(ss, line, '\n'))
	{
		std::stringstream	line_ss(line);
		std::string			command_type;
		
		line_ss >> command_type;
		if (command_type == "NICK")
		{
			Users.addnick(line_ss, cur_event);
		}
		else if (command_type == "USER") 
		{
			struct Udata	*u_data = Users.adduser(line_ss, cur_event.ident);
			kq_.SetWrite(cur_event.ident, u_data);
		}
		else if (command_type == "PING")
		{
			std::string serv_add;

			line_ss >> serv_add;

			//struct Udata	*u_data = Sender::pong(cur_event.ident, serv_add);
			kq_.SetWrite(cur_event.ident, u_data);
			// Sender::pong(cur_event.ident, serv_add); // MOVE TO WRITE PART
		}
		// else if (command_type == "PRIVMSG")
		// {
		// 	std::string target, msg;
		// 	line_ss >> target >> msg;

		// 	//1. send한 user identify
		// 	user sender = Users.search_user_event(cur_event[i]);

		// 	//2. receive할 user find
		// 	user receiver = Users.search_user_nick(target);

		// 	//여기서 write로 바꿔버린 다음에 보내면 안되냐?
		// 	// receive의 Event를 write
		// 	if (receiver.client_sock_ == -433)
		// 	{
		// 		Sender::send_err(sender, receiver, msg);
		// 	}
		// 	else
		// 	{
		// 		Sender::privmsg(sender, receiver, msg);
		// 	}
		// 	//Read로 바꿈 되잖아 
		// }
	}
	return (0);
}

int	Receiver::clientWriteEventHandler(struct kevent &cur_event)
{
	//struct udata	*u_data = reinterpret_cast<udata *>(cur_event.udata);
	Udata *udata = static_cast<Udata*>(cur_event.udata);

	std::cout << "socket: " << udata->sock_fd << " msg: " << udata->msg << std::endl;
	send(udata->sock_fd, udata->msg.c_str(), udata->msg.length(), 0);
	return (1);
}