#include "KeventHandler.hpp"

void exit_with_perror(const std::string& msg);

/**		keventHandler   **/
/**		@brief kqueue를 새로 등록   **/
KeventHandler::KeventHandler()
{
	kq_ = kqueue();
	if (kq_ < 0)
	{
		exit_with_perror("err: Kqueue Creating Fail");
	}
}

KeventHandler::~KeventHandler()
{
	if (kq_ > 0)
	{
		close(kq_);
	}
}

void	KeventHandler::kevent_init_(uintptr_t ident, int16_t filter, uint16_t flags, uint32_t fflags, int64_t data, void *udata)
{
	struct kevent	event = {};

	EV_SET(&event, ident, filter, flags, fflags, data, udata);
	change_list_.push_back(event);
}

/**		set_monitor   **/
/**		@brief 새로 발생한 이벤트들을 kevent 벡터에 담아주는 함수   **/
/**		@param 이벤트 등록을 계속할 지 판별하는 flag   **/
std::vector<struct kevent>	KeventHandler::set_monitor(const bool& end_signal)
{
	std::vector<struct kevent>	res;

	if (end_signal)
		return res;
	struct kevent	event_list[10];
	int	event_num(-1);

	while ((event_num < 0))
	{
		event_num = kevent(kq_, &(change_list_[0]), (int)change_list_.size(), event_list, 10, NULL);
	}
	for (int i(0); i < event_num; ++i)
	{
		res.push_back(event_list[i]);
	}
	change_list_.clear();
	return (res);
}

void	KeventHandler::set_server(uintptr_t ident)
{
	kevent_init_(ident, EVFILT_READ, EV_ADD, 0, 0, NULL);
}

void	KeventHandler::set_read(uintptr_t ident)
{
	kevent_init_(ident, EVFILT_READ, EV_ADD | EV_ONESHOT, 0, 0, NULL);
}

void	KeventHandler::set_write(uintptr_t ident)
{
	kevent_init_(ident, EVFILT_WRITE, EV_ADD, 0, 0, NULL);
}

/**		set_exit   **/
/**		@brief set_write와 유사하나 종료 신호를 보내기 위한 더미 udata를 생성하여 이벤트를 등록   **/
void	KeventHandler::set_exit(uintptr_t ident)
{
	char	k(0);

	kevent_init_(ident, EVFILT_WRITE, EV_ADD, 0, 0, &k);
}

void	KeventHandler::delete_event(const struct kevent &event)
{
	struct kevent	tmp;

	EV_SET(&tmp, event.ident, event.filter, EV_DELETE, 0, 0, 0);
	kevent(kq_, &tmp, 1, NULL, 0, NULL);
}

void	KeventHandler::delete_server(uintptr_t serv_sock)
{
	struct kevent	tmp;

	EV_SET(&tmp, serv_sock, EVFILT_READ, EV_DELETE, 0, 0, 0);
	kevent(kq_, &tmp, 1, NULL, 0, NULL);
}
