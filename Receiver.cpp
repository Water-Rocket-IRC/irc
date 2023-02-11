#include "Receiver.hpp"
#include "Database.hpp"
#include "Udata.hpp"
#include "color.hpp"
#include <atomic>
#include <string>
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

/**		carriage_remover   **/
/**		@brief 클라이언트로 부터 읽은 데이터에서 캐리지 리턴(\r)을 없애주는 함수   **/
/**		@param to_remove 없앨 문자열   **/
void	Receiver::carriage_remover_(std::string& to_remove)
{
	std::size_t	carriage_pos = to_remove.find('\r');

	while (carriage_pos != std::string::npos)
	{
		to_remove.erase(carriage_pos, 1);
		carriage_pos = to_remove.find('\r');
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
		parser_.error_situation(tmp_sock);
		close(tmp_sock);
		return (1);
	}
	int byte_received = recv(cur_event.ident, buffer, sizeof(buffer), 0);
	/**    클라이언트 소켓으로 듣는데 실패한 경우 해당 이벤트를 넘긴다.   **/
	if (byte_received < 0)
	{
		std::cerr << RED << "err: receiving data" << RESET << std::endl;
		return (1);
	}
	/*****   한 줄을 받은 command에서 개행(\n)이 있는지 먼저 찾는다   *****/
	std::string			command(buffer, byte_received);
	std::size_t			newline_pos = command.find('\n');

	/*****   문자열의 모든 캐리지 리턴(\r)을 없앤다.   *****/
	carriage_remover_(command);
	
	/*****   만약 개행을 찾지 못 했다면   *****/
	if (newline_pos == std::string::npos)
	{
		/**   먼저 저장되어 있는 백업이 있는지 찾는다.  **/
		Udata_iter	cur_backup = carriage_backup_.find(cur_event.ident);

		/**   개행이 없고, 백업이 없으면 백업본을 하나 만들어 저장한다.  **/
		if (cur_backup == carriage_backup_.end())
		{
			Event	tmp;
			tmp.first = cur_event.ident;
			tmp.second = command;
			carriage_backup_.insert(tmp);
		}
		/**   백업된 문자열이 있으면 해당 문자열에 추가해준다.  **/
		else
		{
			cur_backup->second += command;
		}
		/**   개행이 나올 때 까지 read 상태로 대기시킨다.  **/
		kq_.set_read(cur_event.ident);
	}
	/*****   개행을 찾았으면 Parser로 커맨드를 넘겨야 한다.  *****/
	else
	{
		/**   먼저 저장되어 있는 백업 본이 있는지 찾는다.  **/
		Udata_iter	cur_backup = carriage_backup_.find(cur_event.ident);

		/**   저장된 백업본이 있으면 클라이언트한테 받은 커맨드 문자열 앞에 백업을 붙여주고, 저장된 데이터는 삭제한다.  **/
		if (cur_backup != carriage_backup_.end())
		{
			command.insert(0, cur_backup->second);
			carriage_backup_.erase(cur_backup);
		}
		/**   Parser를 호출하여 한 줄의 명령어를 처리한다.  **/
		/**   Receiver에서 캐리지 리턴을 모두 제거하여 Parser에서의 캐리지 처리(set_message)함수는 제거되었음  **/
		parser_.command_parser(cur_event.ident, command);
	}
	return (0);
}

/**		client_write_event_handler   **/
/**		@brief client 소켓에게 패킷을 보낼 경우 처리하는 함수   **/
/**		@param cur_event 현재 발생된 이벤트   **/
int	Receiver::client_write_event_handler_(struct kevent &cur_event)
{
	Udata_iter	target = udata_.find(cur_event.ident);

	int	send_bytes = send(cur_event.ident, target->second.c_str(), target->second.size(), 0);
	if (send_bytes < 0)
	{
		std::cerr << RED << "err: sending data" << RESET << std::endl;
		return (1);
	}
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
