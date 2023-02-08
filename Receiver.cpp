#include "Receiver.hpp"
#include "Database.hpp"
#include "Udata.hpp"
#include <sys/_types/_uintptr_t.h>

void exit_with_perror(const std::string& msg);

bool Receiver::end_server = true;
KeventHandler	Receiver::kq_ = KeventHandler(end_server);

KeventHandler&	Receiver::get_Kevent_Handler(void)
{
	return kq_;
}

// class Parser;
/*    Receiver Class     */
/// @brief Receiver 생성자
/// @param port 소켓을 생성할 포켓 번호
Receiver::Receiver(Udata& serv_udata, const uintptr_t& port, const std::string& password)
: parser_(serv_udata, password), udata_(serv_udata)
{
	init_socket_(port);
	bind_socket_();
}

void	Receiver::stop_receiver(void)
{
	// end_server = false;
	parser_.clear_all();
	std::cerr << "Parser clear all end\n";
	close(server_sock_);
	std::cerr << "close end\n";
	kq_.delete_server(server_sock_);
	std::cerr << "Receiver stop_receiver end\n";
}

void	Receiver::init_socket_(const uintptr_t& port)
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
	kq_.set_server(server_sock_);
}

void	Receiver::start()
{
	while (end_server)
	{
		std::vector<struct kevent>	events = kq_.set_monitor();
		for (std::size_t i(0); i < events.size(); ++i)
		{
			struct kevent	cur_event = events[i];	// event occur with new accept
			if (cur_event.ident == server_sock_)
			{
				uintptr_t	client_sock = accept(server_sock_, NULL, NULL);
				if (client_sock < 0)
				{
					std::cerr << "err: accepting connection fail" << std::endl;
					continue ;
				}
				kq_.set_read(client_sock);
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
					if (clientWriteEventHandler_(cur_event))
					{
						continue ;
					}
					uintptr_t	tmp_fd = cur_event.ident;
					kq_.delete_event(cur_event);
					kq_.set_read(tmp_fd);
				}
			}
		}
	}
}

int	Receiver::clientReadEventHandler_(struct kevent &cur_event)
{
	char	buffer[1024];

	memset(buffer, 0, sizeof(buffer));
	if (cur_event.flags & EV_EOF)
	{
		uintptr_t	tmp_sock(cur_event.ident);
		std::cout << tmp_sock << " sock was fucked!" << std::endl;
		kq_.delete_event(cur_event);
		parser_.error_situation(tmp_sock);
		close(tmp_sock);
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

	Udata_iter	target = udata_.find(cur_event.ident);

	std::cout << BOLDGREEN
				<< "socket: " << target->first << "\nmsg:\n" << target->second
				<< RESET << std::endl;
	send(cur_event.ident, target->second.c_str(), target->second.size(), 0);
	if (cur_event.udata)
	{
		uintptr_t	tmp_fd = cur_event.ident;
		kq_.delete_event(cur_event);
		close(tmp_fd); 
		udata_.erase(target);
		return (1);
	}
	udata_.erase(target);
	return (0);
}
