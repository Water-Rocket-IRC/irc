#pragma once

#include "color.hpp"
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
		void						kevent_init_(uintptr_t ident, int16_t filter, uint16_t flags, uint32_t fflags, int64_t data, void *udata);
	public:
		KeventHandler();
		~KeventHandler();

		std::vector<struct kevent>	set_monitor();
		void						set_read(uintptr_t ident);
		void						set_write(uintptr_t ident);
		void						delete_event(const struct kevent &event);
		void						delete_server(uintptr_t serv_sock);
};
