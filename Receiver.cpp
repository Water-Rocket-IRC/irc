#include "Receiver.hpp"

KeventHandler	Receiver::get_Kevent_Handler(void)
{
	return kq_;
}

// class Parser;
/*    Receiver Class     */
/// @brief Receiver 생성자
/// @param port 소켓을 생성할 포켓 번호
Receiver::Receiver(int port)
{
	init_socket_(port);
	bind_socket_();
}

Receiver::~Receiver()
{
	close(server_sock_);
}

void	Receiver::init_socket_(int &port)
{
	server_sock_ = socket(AF_INET, SOCK_STREAM, 0);
	if (server_sock_ < 0)
	{
		exit_with_perror("err: Socket Creating Fail");
	}
	server_addr_.sin_family = AF_INET;
	server_addr_.sin_port = htons(port);
	server_addr_.sin_addr.s_addr = INADDR_ANY;
}

void	Receiver::bind_socket_()
{
	// socket bind
	if (bind(server_sock_, (sockaddr *) &server_addr_, sizeof(server_addr_)) < 0)
	{
		exit_with_perror("err: Socket Binding Fail");
	}
	if (listen(server_sock_, 5) < 0)
	{
		std::cerr << "error" << std::endl;
	}
	// TODO: Have to arrange 5 (Max queue)
	kq_.set_read(server_sock_);
}

void	Receiver::start()
{
	while (true)
	{
		std::vector<struct kevent>	events = kq_.set_monitor();
		for (std::size_t i(0); i < events.size(); ++i)
		{
			struct kevent	cur_event = events[i];	// event occur with new accept
			if (cur_event.ident == server_sock_)
			{
				client_sock_ = accept(server_sock_, NULL, NULL);
				if (client_sock_ < 0)
				{
					std::cerr << "err: accepting connection fail" << std::endl;
					continue ;
				}
				kq_.set_read(client_sock_);
			}
			else	// event occur with users
			{
				if (cur_event.filter == EVFILT_READ)
				{
					if (clientReadEventHandler_(cur_event))
					{
						continue ;
					}
				}
				else if (cur_event.filter == EVFILT_WRITE)
				{
					// TODO: LEAK CHECK
					if (clientWriteEventHandler_(cur_event))
					{
						int	tmp_fd = cur_event.ident;
						kq_.delete_event(cur_event);
						kq_.set_read(tmp_fd);
					}
				}
			}
		}
	}
}

int	Receiver::clientReadEventHandler_(struct kevent &cur_event)
{
	char	buffer[1024];


	memset(buffer, 0, sizeof(buffer));
	// TODO: LEAK CHECK
	if (cur_event.flags & EV_EOF)
	{
		std::cout << "sock was fucked!" << std::endl;
		kq_.delete_event(cur_event);
		// TODO: delete User
		return (1);
	}
	int byte_received = recv(cur_event.ident, buffer, sizeof(buffer), 0);
	if (byte_received < 0)
	{
		std::cerr << "err: receiving data" << std::endl;
		return (1);
	}
	std::cout << BOLDYELLOW << "Received: " << cur_event.ident << "\n" << buffer << RESET<< std::endl;
	std::string			command(buffer, byte_received);

	 parser_.command_parser(cur_event.ident, command);
	return (0);
}

int	Receiver::clientWriteEventHandler_(struct kevent &cur_event)
{
	if (udata_.size())
	{
		for (std::size_t i(0); i < udata_.size(); ++i)
		{
			if (udata_[i].sock_fd == cur_event.ident)
			{
				std::cout << BOLDGREEN
						  << "socket: " << udata_[i].sock_fd << "\nmsg: " << udata_[i].msg
						  << RESET << std::endl;
				send(cur_event.ident, udata_[i].msg.c_str(), udata_[i].msg.length(), 0);
				udata_.erase(udata_.begin() + i);
			}
		}
		return (1);
	}
	return (0);
}
