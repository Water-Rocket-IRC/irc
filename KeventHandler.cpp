#include "KeventHandler.hpp"

void exit_with_perror(const std::string& msg);

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

std::vector<struct kevent>	KeventHandler::set_monitor()
{
	struct kevent	event_list[10];
	std::vector<struct kevent>	res;

	int	event_num = -1;
	while (event_num < 0)
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

void	KeventHandler::set_exit(uintptr_t ident, void* flag)
{
	kevent_init_(ident, EVFILT_WRITE, EV_ADD, 0, 0, flag);
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
