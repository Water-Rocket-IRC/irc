#pragma once

#include <iostream>
#include <unistd.h>
#include <vector>
#include <sys/event.h>
#include <sys/time.h>

class KeventHandler
{
	private:
		int							kq_;
		
		std::vector<struct kevent>	change_list_;
		void						KeventInit(uintptr_t ident, int16_t filter, uint16_t flags, uint32_t fflags, int64_t data, void *udata);
	public:
		KeventHandler();
		~KeventHandler();

		std::vector<struct kevent>	SetMonitor();
		void	SetRead(uintptr_t ident, void *udata);
		void	SetWrite(uintptr_t ident, void *udata);
		void	DeleteEvent(const struct kevent &event);
		void	get_kq();
};

void	KeventHandler::get_kq()
{
	std::cout << kq_ << std::endl;
}

void exit_with_perror(const std::string& msg)
{
	system("clear");
	std::cerr << msg << std::endl;
	exit(EXIT_FAILURE);
}

KeventHandler::KeventHandler()
{
	kq_ = kqueue();
	if (kq_ < 0)
		exit_with_perror("err: Kqueue Creating Fail");
}

KeventHandler::~KeventHandler()
{
	// if (kq_ > 0)
	// 	close(kq_);
}

void	KeventHandler::KeventInit(uintptr_t ident, int16_t filter, uint16_t flags, uint32_t fflags, int64_t data, void *udata)
{
	struct kevent	event = {};

	EV_SET(&event, ident, filter, flags, fflags, data, udata);
	change_list_.push_back(event);
}

std::vector<struct kevent>	KeventHandler::SetMonitor()
{
	struct kevent	event_list[10];
	std::vector<struct kevent>	res;

	int	event_num = -1;
	while (event_num < 0)
	{
		event_num = kevent(kq_, &(change_list_[0]), (int)change_list_.size(), event_list, 10, NULL);
		// if (errno == EACCES) {
		// 	std::perror("error: ");
		// } else if (errno == EFAULT) {
		// 	std::perror("error: ");
		// } else if (errno == EBADF) {
		// 	std::perror("error: ");
		// } else if (errno == EINTR) {
		// 	std::perror("error: ");
		// } else if (errno == EINVAL) {
		// 	std::perror("error: ");
		// } else if (errno == ENOENT) {
		// 	std::perror("error: ");
		// } else if (errno == ENOMEM) {
		// 	std::perror("error: ");
		// } else if (errno == ESRCH) {
		// 	std::perror("error: ");
		// }
		// std::cout << event_num << std::endl;
		// std::cout << "wtf??" << std::endl;
	}
	for (int i(0); i < event_num; ++i)
		res.push_back(event_list[i]);
	change_list_.clear();
	return (res);
}

void	KeventHandler::SetRead(uintptr_t ident, void *udata)
{
	KeventInit(ident, EVFILT_READ, EV_ADD, 0, 0, udata);
}

void	KeventHandler::SetWrite(uintptr_t ident, void *udata)
{
	KeventInit(ident, EVFILT_WRITE, EV_ADD, 0, 0, udata);
}

void	KeventHandler::DeleteEvent(const struct kevent &event)
{
	struct kevent	tmp;
	EV_SET(&tmp, event.ident, event.filter, EV_DELETE, 0, 0, 0);
	kevent(kq_, &tmp, 1, NULL, 0, NULL);
}
