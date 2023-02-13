#include "Receiver.hpp"
#include "Database.hpp"
#include "Udata.hpp"
#include <sys/_types/_uintptr_t.h>

void exit_with_perror(const std::string& msg);

KeventHandler	Receiver::kq_ = KeventHandler();

KeventHandler&	Receiver::get_Kevent_Handler(void)
{
	return kq_;
}

/**		Receiver Constructor   **/
/**		@brief Receiver 생성자   **/
/**		@param serv_udata 이벤트를 관리할 map   **/
/**		@param port 소켓을 생성할 포켓 번호   **/
/**		@param password irc서버 비밀번호   **/
Receiver::Receiver(Udata& serv_udata, const uintptr_t& port, const std::string& password)
: parser_(serv_udata, password), udata_(serv_udata)
{
	init_socket_(port);
	bind_socket_();
}

void	Receiver::stop_receiver(void)
{
	parser_.clear_all();
	close(server_sock_);
	kq_.delete_server(server_sock_);
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
	if (bind(server_sock_, (sockaddr *) &server_addr_, sizeof(server_addr_)) < 0)
	{
		exit_with_perror("err: Socket Binding Fail");
	}
	if (listen(server_sock_, 5) < 0)
	{
		std::cerr << "error" << std::endl;
	}
	kq_.set_server(server_sock_);
}

void	Receiver::start()
{
	while (true)
	{
		std::vector<struct kevent>	events = kq_.set_monitor(false);
		for (std::size_t i(0); i < events.size(); ++i)
		{
			struct kevent	cur_event = events[i];
			/**   서버의 소켓으로 이벤트가 발생한 경우(클라이언트가 처음 접속)   **/
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
			/**   클라이언트 소켓으로 이벤트가 발생한 경우   **/
			else
			{
				/**   클라이언트가 서버로 패킷을 보낼 경우   **/
				if (cur_event.filter == EVFILT_READ)
				{
					/**   클라이언트 소켓이 비정상 종료된 경우 패스   **/
					if (client_read_event_handler_(cur_event))
					{
						continue ;
					}
				}
				/**   서버가 클라이언트로 패킷을 보낼 경우   **/
				else if (cur_event.filter == EVFILT_WRITE)
				{
					/**   클라이언트 소켓이 정상 종료된 경우 패스   **/
					if (client_write_event_handler_(cur_event))
					{
						continue ;
					}
					/**   Write 이벤트를 다시 Read 상태로 변경   **/
					uintptr_t	tmp_fd = cur_event.ident;
					kq_.delete_event(cur_event);
					kq_.set_read(tmp_fd);
				}
			}
		}
	}
}

/**		client_read_event_handler   **/
/**		@brief client 소켓으로 부터 패킷이 들어올 경우 처리하는 함수   **/
/**		@param cur_event 현재 발생된 이벤트   **/
int	Receiver::client_read_event_handler_(struct kevent &cur_event)
{
	char	buffer[1024];

	memset(buffer, 0, sizeof(buffer));
	/**    클라이언트 소켓이 비정상 종료된 경우 해당 클라이언트 정보를 지우고 소켓도 닫는다.   **/
	if (cur_event.flags & EV_EOF)
	{
		uintptr_t	tmp_sock(cur_event.ident);

		kq_.delete_event(cur_event);
		std::cerr << "sock was unexpected situation\n";
		parser_.error_situation(tmp_sock);
		close(tmp_sock);
		return (1);
	}
	int byte_received = recv(cur_event.ident, buffer, sizeof(buffer), 0);
	/**    클라이언트 소켓으로 듣는데 실패한 경우 해당 이벤트를 넘긴다.   **/
	if (byte_received < 0)
	{
		std::cerr << "err: receiving data" << std::endl;
		return (1);
	}
	std::string			command(buffer, byte_received);

	parser_.command_parser(cur_event.ident, command);
	return (0);
}

/**		client_write_event_handler   **/
/**		@brief client 소켓에게 패킷을 보낼 경우 처리하는 함수   **/
/**		@param cur_event 현재 발생된 이벤트   **/
int	Receiver::client_write_event_handler_(struct kevent &cur_event)
{
	Udata_iter	target = udata_.find(cur_event.ident);

	send(cur_event.ident, target->second.c_str(), target->second.size(), 0);
	/**   해당 클라이언트가 종료를 요청한 경우 클라이언트 삭제 처리   **/
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
